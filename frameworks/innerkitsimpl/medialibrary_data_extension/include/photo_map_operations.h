/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_PHOTO_MAP_OPERATIONS_H
#define OHOS_PHOTO_MAP_OPERATIONS_H

#include "datashare_values_bucket.h"
#include "rdb_predicates.h"

namespace OHOS::Media {
class PhotoMapOperations {
public:
    static int32_t AddPhotoAssets(const std::vector<DataShare::DataShareValuesBucket> &values);
    static int32_t RemovePhotoAssets(NativeRdb::RdbPredicates &predicates);
};
} // namespace OHOS::Media
#endif // OHOS_PHOTO_MAP_OPERATIONS_H
