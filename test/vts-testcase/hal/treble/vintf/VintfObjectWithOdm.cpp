/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "VintfObjectWithOdm.h"

#include <vintf/CompatibilityMatrix.h>
#include <vintf/parse_xml.h>
#include <vintf/VintfObject.h>
#include "utils.h"

#include <android-base/properties.h>

#include <functional>
#include <memory>
#include <mutex>

namespace android {
namespace vintf_with_odm {
using namespace vintf;

template <typename T>
struct LockedUniquePtr {
    std::unique_ptr<T> object;
    std::mutex mutex;
};

static LockedUniquePtr<HalManifest> gProductManifest;
static LockedUniquePtr<HalManifest> gOdmManifest;
static LockedUniquePtr<HalManifest> gVendorManifest;
static std::mutex gDeviceManifestMutex;

static LockedUniquePtr<CompatibilityMatrix> gDeviceMatrix;
static LockedUniquePtr<CompatibilityMatrix> gFrameworkMatrix;
static LockedUniquePtr<RuntimeInfo> gDeviceRuntimeInfo;

template <typename T, typename F>
static const T *Get(
        LockedUniquePtr<T> *ptr,
        bool skipCache,
        const F &fetchAllInformation) {
    std::unique_lock<std::mutex> _lock(ptr->mutex);
    if (skipCache || ptr->object == nullptr) {
        ptr->object = std::make_unique<T>();
        if (fetchAllInformation(ptr->object.get()) != OK) {
            ptr->object = nullptr; // frees the old object
        }
    }
    return ptr->object.get();
}

status_t fetchAllInformation(HalManifest* manifest, const std::string &path) {
    return vintf::details::fetchAllInformation(path, gHalManifestConverter, manifest);
}

// static
const HalManifest *VintfObject::GetDeviceHalManifest(bool skipCache) {
    std::unique_lock<std::mutex> _lock(gDeviceManifestMutex);

    std::string productModel = android::base::GetProperty("ro.boot.product.hardware.sku", "");
    if (!productModel.empty()) {
        auto product = Get(&gProductManifest, skipCache,
                           std::bind(&fetchAllInformation, std::placeholders::_1,
                                     "/odm/etc/manifest_" + productModel + ".xml"));
        if (product != nullptr) {
            return product;
        }
    }

    auto odm = Get(
        &gOdmManifest, skipCache,
        std::bind(&fetchAllInformation, std::placeholders::_1, "/odm/etc/manifest.xml"));
    if (odm != nullptr) {
        return odm;
    }

    return Get(&gVendorManifest, skipCache,
               std::bind(&fetchAllInformation, std::placeholders::_1,
                         "/vendor/manifest.xml"));
}

// static
const HalManifest *VintfObject::GetFrameworkHalManifest(bool skipCache) {
    return vintf::VintfObject::GetFrameworkHalManifest(skipCache);
}


// static
const CompatibilityMatrix *VintfObject::GetDeviceCompatibilityMatrix(bool skipCache) {
    return vintf::VintfObject::GetDeviceCompatibilityMatrix(skipCache);
}

// static
const CompatibilityMatrix *VintfObject::GetFrameworkCompatibilityMatrix(bool skipCache) {
    return vintf::VintfObject::GetFrameworkCompatibilityMatrix(skipCache);
}

// static
const RuntimeInfo *VintfObject::GetRuntimeInfo(bool skipCache) {
    return vintf::VintfObject::GetRuntimeInfo(skipCache);
}

// static
int32_t VintfObject::CheckCompatibility(const std::vector<std::string>& xmls, std::string* error,
                                        DisabledChecks disabledChecks) {
    return vintf::VintfObject::CheckCompatibility(xmls, error, disabledChecks);
}


} // namespace vintf
} // namespace android
