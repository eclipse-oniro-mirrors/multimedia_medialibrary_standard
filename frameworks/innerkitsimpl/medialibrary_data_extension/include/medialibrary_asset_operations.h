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

#ifndef MEDIALIBRARY_FILE_OPERATIONS
#define MEDIALIBRARY_FILE_OPERATIONS

#include <memory>
#include <string>
#include <vector>

#include "abs_shared_result_set.h"
#include "medialibrary_command.h"

namespace OHOS {
namespace Media {
class MediaLibraryAssetOperations {
public:
    static int32_t HandleInsertOperation(MediaLibraryCommand &cmd);
    static int32_t CreateOperation(MediaLibraryCommand &cmd);
    static int32_t DeleteOperation(MediaLibraryCommand &cmd);
    static std::shared_ptr<NativeRdb::ResultSet> QueryOperation(MediaLibraryCommand &cmd,
        const std::vector<std::string> &columns);
    static int32_t UpdateOperation(MediaLibraryCommand &cmd);
    static int32_t OpenOperation(MediaLibraryCommand &cmd, const std::string &mode);
    static int32_t CloseOperation(MediaLibraryCommand &cmd);
};

} // namespace Media
} // namespace OHOS

#endif // MEDIALIBRARY_FILE_OPERATIONS