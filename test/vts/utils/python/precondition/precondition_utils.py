#
# Copyright (C) 2017 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import logging

from vts.runners.host import const
from vts.runners.host import keys
from vts.utils.python.common import vintf_utils


HAL_INTERFACE_DELIMITER = "::"


def FindHalDescription(hal_desc, hal_package_name):
    """Find a HAL description whose name is hal_package_name from hal_desc."""
    for hal_full_name in hal_desc:
        if hal_desc[hal_full_name].hal_name == hal_package_name:
            return hal_desc[hal_full_name]
    return None


def IsHalRegisteredInVintfXml(hal, vintf_xml, bitness):
    """Checks whether a HAL is registered in a VINTF XML.

    If the given hal is an earlier minor version of what is specified in
    vintf_xml, it returns True.

    Args:
        hal: string, the full name of a HAL (e.g., package@version)
        vintf_xml: string, the VINTF XML content.
        bitness, string, currently tested ABI bitness (e.g., 32 or 64).

    Returns:
        True if found or vintf_xml is malformed, False otherwise.
    """
    if "@" not in hal:
        logging.error("HAL full name is invalid, %s", hal)
        return False
    hal_package, hal_version = hal.split("@")

    items = hal_version.split(HAL_INTERFACE_DELIMITER)
    if len(items) > 2:
        logging.error("precondition HAL name format error: %s", hal)
        return False
    elif len(items) == 2:
        hal_version, interface = items
    else:
        interface = None

    logging.info("HAL package, version = %s, %s", hal_package, hal_version)
    hal_version_major, hal_version_minor = vintf_utils.ParseHalVersion(
        hal_version)

    hwbinder_hals, passthrough_hals = vintf_utils.GetHalDescriptions(
        vintf_xml)
    hwbinder_hal_desc = FindHalDescription(hwbinder_hals, hal_package)
    passthrough_hal_desc = FindHalDescription(passthrough_hals, hal_package)
    if not hwbinder_hals or not passthrough_hals:
        logging.error("can't check precondition due to a "
                  "VINTF XML format error.")
        # Assume it's satisfied.
        return True
    elif (hwbinder_hal_desc is None and passthrough_hal_desc is None):
        logging.warn(
            "The required HAL %s not found in VINTF XML.", hal)
        return False
    else:
        if passthrough_hal_desc:
            if bitness:
                if (bitness not in passthrough_hal_desc.hal_archs):
                    logging.warn(
                        "The required feature %s found as a "
                        "passthrough HAL but the client bitness %s "
                        "unsupported",
                        hal, bitness)
                    return False
            hal_desc = passthrough_hal_desc
        else:
            hal_desc = hwbinder_hal_desc

        if interface:
            has_interface = False
            for hal_interface in hal_desc.hal_interfaces:
                if hal_interface.hal_interface_name == interface:
                    has_interface = True
                    break

            if not has_interface:
                logging.warn("The required HAL interface %s is not "
                             "available for HAL %s", interface, hal)
                return False

        logging.info(
            "The feature %s found in VINTF XML", hal)

    found_version_major = hal_desc.hal_version_major
    found_version_minor = hal_desc.hal_version_minor
    if (hal_version_major != found_version_major or
        hal_version_minor > found_version_minor):
        logging.warn(
            "The found HAL version %s@%s is not relevant for %s",
            found_version_major, found_version_minor, hal_version)
        return False

    return True


def CanRunHidlHalTest(test_instance, dut, shell=None):
    """Checks HAL precondition of a test instance.

    Args:
        test_instance: the test instance which inherits BaseTestClass.
        dut: the AndroidDevice under test.
        shell: the ShellMirrorObject to execute command on the device.
               If not specified, the function creates one from dut.

    Returns:
        True if the precondition is satisfied; False otherwise.
    """
    if shell is None:
        dut.shell.InvokeTerminal("check_hal_preconditions")
        shell = dut.shell.check_hal_preconditions

    opt_params = [
        keys.ConfigKeys.IKEY_ABI_BITNESS,
        keys.ConfigKeys.IKEY_PRECONDITION_HWBINDER_SERVICE,
        keys.ConfigKeys.IKEY_PRECONDITION_FEATURE,
        keys.ConfigKeys.IKEY_PRECONDITION_FILE_PATH_PREFIX,
        keys.ConfigKeys.IKEY_PRECONDITION_LSHAL,
        keys.ConfigKeys.IKEY_PRECONDITION_VINTF,
    ]
    test_instance.getUserParams(opt_param_names=opt_params)

    hwbinder_service_name = str(getattr(test_instance,
        keys.ConfigKeys.IKEY_PRECONDITION_HWBINDER_SERVICE, ""))
    if hwbinder_service_name:
        if not hwbinder_service_name.startswith("android.hardware."):
            logging.error("The given hwbinder service name %s is invalid.",
                          hwbinder_service_name)
        else:
            cmd_results = shell.Execute("ps -A")
            hwbinder_service_name += "@"
            if (any(cmd_results[const.EXIT_CODE]) or
                hwbinder_service_name not in cmd_results[const.STDOUT][0]):
                logging.warn("The required hwbinder service %s not found.",
                             hwbinder_service_name)
                return False

    feature = str(getattr(test_instance,
        keys.ConfigKeys.IKEY_PRECONDITION_FEATURE, ""))
    if feature:
        if not feature.startswith("android.hardware."):
            logging.error(
                "The given feature name %s is invalid for HIDL HAL.",
                feature)
        else:
            cmd_results = shell.Execute("LD_LIBRARY_PATH= pm list features")
            if (any(cmd_results[const.EXIT_CODE]) or
                feature not in cmd_results[const.STDOUT][0]):
                logging.warn("The required feature %s not found.",
                             feature)
                return False

    file_path_prefix = str(getattr(test_instance,
        keys.ConfigKeys.IKEY_PRECONDITION_FILE_PATH_PREFIX, ""))
    if file_path_prefix:
        cmd_results = shell.Execute("ls %s*" % file_path_prefix)
        if any(cmd_results[const.EXIT_CODE]):
            logging.warn("The required file (prefix: %s) not found.",
                         file_path_prefix)
            return False

    hal = str(getattr(test_instance,
        keys.ConfigKeys.IKEY_PRECONDITION_VINTF, ""))
    vintf_xml = None
    if hal:
        use_lshal = False
        vintf_xml = dut.getVintfXml(use_lshal=use_lshal)
        logging.debug("precondition-vintf used to retrieve VINTF xml.")
    else:
        use_lshal = True
        hal = str(getattr(test_instance,
            keys.ConfigKeys.IKEY_PRECONDITION_LSHAL, ""))
        if hal:
            vintf_xml = dut.getVintfXml(use_lshal=use_lshal)
            logging.debug("precondition-lshal used to retrieve VINTF xml.")

    if vintf_xml:
        result = IsHalRegisteredInVintfXml(hal, vintf_xml,
                                           test_instance.abi_bitness)
        if not result and use_lshal:
            # this is for when a test is configured to use the runtime HAL
            # service availability (the default mode for HIDL tests).
            # if a HAL is in vendor/manifest.xml, test is supposed to fail
            # even though a respective HIDL HAL service is not running.
            vintf_xml = dut.getVintfXml(use_lshal=False)
            return IsHalRegisteredInVintfXml(hal, vintf_xml,
                                             test_instance.abi_bitness)
        return result

    return True


def CheckSysPropPrecondition(test_instance,
                             dut,
                             shell=None):
    """Checks sysprop precondition of a test instance.

    Args:
        test_instance: the test instance which inherits BaseTestClass.
        dut: the AndroidDevice under test.
        shell: the ShellMirrorObject to execute command on the device.
               If not specified, the function creates one from dut.

    Returns:
        False if precondition is not met (i.e., to skip tests),
        True otherwise (e.g., when no sysprop precondition is set;
        the precondition is satisfied;
        there is an error in retrieving the target sysprop; or
        the specified sysprop is undefined)
    """
    if not hasattr(test_instance, keys.ConfigKeys.IKEY_PRECONDITION_SYSPROP):
        return True

    precond_sysprop = str(getattr(
        test_instance, keys.ConfigKeys.IKEY_PRECONDITION_SYSPROP, ''))
    if "=" not in precond_sysprop:
        logging.error("precondition-sysprop value is invalid.")
        return True

    if shell is None:
        dut.shell.InvokeTerminal("check_sysprop_precondition")
        shell = dut.shell.check_sysprop_precondition

    sysprop_key, sysprop_value = precond_sysprop.split('=')
    cmd_results = shell.Execute('getprop %s' % sysprop_key)
    if any(cmd_results[const.EXIT_CODE]):
        logging.error('Failed to read sysprop:\n%s', sysprop_key)
        return True
    else:
        value = cmd_results[const.STDOUT][0].strip()
        if len(value) == 0:
            return True
        elif value != sysprop_value:
            return False
    return True
