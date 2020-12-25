#!/usr/bin/python

# This script is used to update platform SDK prebuilts, Support Library, and a variety of other
# prebuilt libraries used by Android's Makefile builds. For details on how to use this script,
# visit go/update-prebuilts.
import os, sys, getopt, zipfile, re
import argparse
import subprocess
from shutil import copyfile, rmtree
from distutils.version import LooseVersion

current_path = 'current'
system_path = 'system_current'
support_dir = os.path.join(current_path, 'support')
extras_dir = os.path.join(current_path, 'extras')

# See go/fetch_artifact for details on this script.
FETCH_ARTIFACT = '/google/data/ro/projects/android/fetch_artifact'

# Does not import support-v4, which is handled as a separate Android.mk (../support-v4) to
# statically include dependencies. Similarly, the support-v13 module is imported as
# support-v13-nodeps and then handled as a separate Android.mk (../support-v13) to statically
# include dependencies.
maven_to_make = {
    'animated-vector-drawable':     ['android-support-animatedvectordrawable',      'graphics/drawable'],
    'appcompat-v7':                 ['android-support-v7-appcompat-nodeps',         'v7/appcompat'],
    'cardview-v7':                  ['android-support-v7-cardview',                 'v7/cardview'],
    'customtabs':                   ['android-support-customtabs',                  'customtabs'],
    'design':                       ['android-support-design',                      'design'],
    'exifinterface':                ['android-support-exifinterface',               'exifinterface'],
    'gridlayout-v7':                ['android-support-v7-gridlayout',               'v7/gridlayout'],
    'leanback-v17':                 ['android-support-v17-leanback',                'v17/leanback'],
    'mediarouter-v7':               ['android-support-v7-mediarouter',              'v7/mediarouter'],
    'multidex':                     ['android-support-multidex',                    'multidex/library'],
    'multidex-instrumentation':     ['android-support-multidex-instrumentation',    'multidex/instrumentation'],
    'palette-v7':                   ['android-support-v7-palette',                  'v7/palette'],
    'percent':                      ['android-support-percent',                     'percent'],
    'preference-leanback-v17':      ['android-support-v17-preference-leanback',     'v17/preference-leanback'],
    'preference-v14':               ['android-support-v14-preference',              'v14/preference'],
    'preference-v7':                ['android-support-v7-preference',               'v7/preference'],
    'recommendation':               ['android-support-recommendation',              'recommendation'],
    'recyclerview-v7':              ['android-support-v7-recyclerview',             'v7/recyclerview'],
    'support-annotations':          ['android-support-annotations',                 'annotations'],
    'support-compat':               ['android-support-compat',                      'compat'],
    'support-core-ui':              ['android-support-core-ui',                     'core-ui'],
    'support-core-utils':           ['android-support-core-utils',                  'core-utils'],
    'support-dynamic-animation':    ['android-support-dynamic-animation',           'dynamic-animation'],
    'support-emoji':                ['android-support-emoji',                       'emoji'],
    'support-emoji-appcompat':      ['android-support-emoji-appcompat',             'emoji-appcompat'],
    'support-emoji-bundled':        ['android-support-emoji-bundled',               'emoji-bundled'],
    'support-fragment':             ['android-support-fragment',                    'fragment'],
    'support-media-compat':         ['android-support-media-compat',                'media-compat'],
    'support-tv-provider':          ['android-support-tv-provider',                 'tv-provider'],
    'support-v13':                  ['android-support-v13-nodeps',                  'v13'],
    'support-vector-drawable':      ['android-support-vectordrawable',              'graphics/drawable'],
    'transition':                   ['android-support-transition',                  'transition'],
    'wear':                         ['android-support-wear',                        'wear'],
    'constraint-layout':            ['android-support-constraint-layout',           'constraint-layout'],
    'constraint-layout-solver':     ['android-support-constraint-layout-solver',    'constraint-layout-solver']
}

# Always remove these files.
blacklist_files = [
    'annotations.zip',
    'public.txt',
    'R.txt',
    'AndroidManifest.xml',
    os.path.join('libs','noto-emoji-compat-java.jar')
]

artifact_pattern = re.compile(r"^(.+?)-(\d+\.\d+\.\d+(?:-\w+\d+)?(?:-[\d.]+)*)\.(jar|aar)$")


def touch(fname, times=None):
    with open(fname, 'a'):
        os.utime(fname, times)


def path(*path_parts):
    return reduce((lambda x, y: os.path.join(x, y)), path_parts)


def flatten(list):
    return reduce((lambda x, y: "%s %s" % (x, y)), list)


def rm(path):
    if os.path.isdir(path):
        rmtree(path)
    elif os.path.exists(path):
        os.remove(path)


def mv(src_path, dst_path):
    if os.path.exists(dst_path):
        rm(dst_path)
    if not os.path.exists(os.path.dirname(dst_path)):
        os.makedirs(os.path.dirname(dst_path))
    os.rename(src_path, dst_path)


def detect_artifacts(repo_dir):
    maven_lib_info = {}

    # Find the latest revision for each artifact.
    for root, dirs, files in os.walk(repo_dir):
        for file in files:
            matcher = artifact_pattern.match(file)
            if matcher:
                maven_lib_name = matcher.group(1)
                maven_lib_vers = LooseVersion(matcher.group(2))

                if maven_lib_name in maven_to_make:
                    if maven_lib_name not in maven_lib_info \
                            or maven_lib_vers > maven_lib_info[maven_lib_name][0]:
                        maven_lib_info[maven_lib_name] = [maven_lib_vers, root, file]

    return maven_lib_info


def transform_maven_repo(repo_dir, update_dir, use_make_dir=True):
    maven_lib_info = detect_artifacts(repo_dir)

    cwd = os.getcwd()

    # Use a temporary working directory.
    working_dir = os.path.join(cwd, 'support_tmp')
    if os.path.exists(working_dir):
        rmtree(working_dir)
    os.mkdir(working_dir)

    for info in maven_lib_info.values():
        transform_maven_lib(working_dir, info[1], info[2], use_make_dir)

    # Replace the old directory.
    output_dir = os.path.join(cwd, update_dir)
    mv(working_dir, output_dir)


def transform_maven_lib(working_dir, root, file, use_make_dir):
    matcher = artifact_pattern.match(file)
    maven_lib_name = matcher.group(1)
    maven_lib_vers = matcher.group(2)
    maven_lib_type = matcher.group(3)

    make_lib_name = maven_to_make[maven_lib_name][0]
    make_dir_name = maven_to_make[maven_lib_name][1]
    artifact_file = os.path.join(root, file)
    target_dir = os.path.join(working_dir, make_dir_name) if use_make_dir else working_dir
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    if maven_lib_type == "aar":
        process_aar(artifact_file, target_dir, make_lib_name)
    else:
        target_file = os.path.join(target_dir, make_lib_name + ".jar")
        os.rename(artifact_file, target_file)

    print maven_lib_vers, ":", maven_lib_name, "->", make_lib_name


def process_aar(artifact_file, target_dir, make_lib_name):
    # Extract AAR file to target_dir.
    with zipfile.ZipFile(artifact_file) as zip:
        zip.extractall(target_dir)

    # Rename classes.jar to match the make artifact
    classes_jar = os.path.join(target_dir, "classes.jar")
    if os.path.exists(classes_jar):
        # If it has resources, it needs a libs dir.
        res_dir = os.path.join(target_dir, "res")
        if os.path.exists(res_dir) and os.listdir(res_dir):
            libs_dir = os.path.join(target_dir, "libs")
            if not os.path.exists(libs_dir):
                os.mkdir(libs_dir)
        else:
            libs_dir = target_dir
        target_jar = os.path.join(libs_dir, make_lib_name + ".jar")
        os.rename(classes_jar, target_jar)

    # Remove or preserve empty dirs.
    for root, dirs, files in os.walk(target_dir):
        for dir in dirs:
            dir_path = os.path.join(root, dir)
            if not os.listdir(dir_path):
                os.rmdir(dir_path)

    # Remove top-level cruft.
    for file in blacklist_files:
        file_path = os.path.join(target_dir, file)
        if os.path.exists(file_path):
            os.remove(file_path)


def fetch_artifact(target, build_id, artifact_path):
    print 'Fetching %s from %s...' % (artifact_path, target)
    fetch_cmd = [FETCH_ARTIFACT, '--bid', str(build_id), '--target', target, artifact_path]
    try:
        subprocess.check_output(fetch_cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError:
        print >> sys.stderr, 'FAIL: Unable to retrieve %s artifact for build ID %d' % (artifact_path, build_id)
        return None
    return artifact_path


def fetch_and_extract(target, build_id, file):
    artifact_path = fetch_artifact(target, build_id, file)
    if not artifact_path:
        return None

    # Unzip the repo archive into a separate directory.
    repo_dir = os.path.basename(artifact_path)[:-4]
    with zipfile.ZipFile(artifact_path) as zipFile:
        zipFile.extractall(repo_dir)

    return repo_dir


def update_support(target, build_id):
    repo_file = 'top-of-tree-m2repository-%s.zip' % build_id
    repo_dir = fetch_and_extract(target, build_id, repo_file)
    if not repo_dir:
        print >> sys.stderr, 'Failed to extract Support Library repository'
        return False

    # Transform the repo archive into a Makefile-compatible format.
    transform_maven_repo(repo_dir, support_dir)
    return True


def update_constraint(target, build_id):
    layout_dir = fetch_and_extract(target, build_id, 'com.android.support.constraint-constraint-layout-%s.zip' % build_id)
    solver_dir = fetch_and_extract(target, build_id, 'com.android.support.constraint-constraint-layout-solver-%s.zip' % build_id)
    if not layout_dir or not solver_dir:
        return False

    # Passing False here is an inelegant solution, but it means we can replace
    # the top-level directory without worrying about other child directories.
    transform_maven_repo(layout_dir, os.path.join(extras_dir, 'constraint-layout'), False)
    transform_maven_repo(solver_dir, os.path.join(extras_dir, 'constraint-layout-solver'), False)
    return True


def extract_to(zip_file, paths, filename, parent_path):
    zip_path = filter(lambda path: filename in path, paths)[0]
    src_path = zip_file.extract(zip_path)
    dst_path = path(parent_path, filename)
    mv(src_path, dst_path)


def update_sdk_repo(target, build_id):
    platform = 'darwin' if 'mac' in target else 'linux'
    artifact_path = fetch_artifact(
        target, build_id, 'sdk-repo-%s-platforms-%s.zip' % (platform, build_id))
    if not artifact_path:
        return False

    with zipfile.ZipFile(artifact_path) as zipFile:
        paths = zipFile.namelist()

        extract_to(zipFile, paths, 'android.jar', current_path)
        extract_to(zipFile, paths, 'uiautomator.jar', current_path)
        extract_to(zipFile, paths, 'framework.aidl', current_path)

        # Unclear if this is actually necessary.
        extract_to(zipFile, paths, 'framework.aidl', system_path)
    return True


def update_system(target, build_id):
    artifact_path = fetch_artifact(target, build_id, 'android_system.jar')
    if not artifact_path:
        return False

    mv(artifact_path, path(system_path, 'android.jar'))
    return True


def append(text, more_text):
    if text:
        return "%s, %s" % (text, more_text)
    return more_text


parser = argparse.ArgumentParser(
    description=('Update current prebuilts'))
parser.add_argument(
    'buildId',
    type=int,
    help='Build server build ID')
parser.add_argument(
    '-c', '--constraint', action="store_true",
    help='If specified, updates only Constraint Layout')
parser.add_argument(
    '-s', '--support', action="store_true",
    help='If specified, updates only the Support Library')
parser.add_argument(
    '-p', '--platform', action="store_true",
    help='If specified, updates only the Android Platform')
args = parser.parse_args()
if not args.buildId:
    parser.error("You must specify a build ID")
    sys.exit(1)
if not (args.support or args.platform or args.constraint):
    parser.error("You must specify at least one of --constraint, --support, or --platform")
    sys.exit(1)

try:
    # Make sure we don't overwrite any pending changes.
    subprocess.check_call(['git', 'diff', '--quiet', '--', '**'])
    subprocess.check_call(['git', 'diff', '--quiet', '--cached', '--', '**'])
except subprocess.CalledProcessError:
    print >> sys.stderr, "FAIL: There are uncommitted changes here; please revert or stash"
    sys.exit(1)

try:
    components = None
    if args.constraint:
        if update_constraint('studio', args.buildId):
            components = append(components, 'Constraint Layout')
            print >> sys.stderr, 'Failed to update Constraint Layout, aborting...'
        else:
            sys.exit(1)
    if args.support:
        if update_support('support_library', args.buildId):
            components = append(components, 'Support Library')
        else:
            print >> sys.stderr, 'Failed to update Support Library, aborting...'
            sys.exit(1)
    if args.platform:
        if update_sdk_repo('sdk_phone_armv7-sdk_mac', args.buildId) \
                and update_system('sdk_phone_armv7-sdk_mac', args.buildId):
            components = append(components, 'platform SDK')
        else:
            print >> sys.stderr, 'Failed to update platform SDK, aborting...'
            sys.exit(1)

    # Commit all changes.
    subprocess.check_call(['git', 'add', current_path])
    subprocess.check_call(['git', 'add', system_path])
    msg = "Import %s from build %s\n\n%s" % (components, args.buildId, flatten(sys.argv))
    subprocess.check_call(['git', 'commit', '-m', msg])
    print 'Remember to test this change before uploading it to Gerrit!'

finally:
    # Revert all stray files, including the downloaded zip.
    try:
        with open(os.devnull, 'w') as bitbucket:
            subprocess.check_call(['git', 'add', '-Af', '.'], stdout=bitbucket)
            subprocess.check_call(
                ['git', 'commit', '-m', 'COMMIT TO REVERT - RESET ME!!!'], stdout=bitbucket)
            subprocess.check_call(['git', 'reset', '--hard', 'HEAD~1'], stdout=bitbucket)
    except subprocess.CalledProcessError:
        print >> sys.stderr, "ERROR: Failed cleaning up, manual cleanup required!!!"
