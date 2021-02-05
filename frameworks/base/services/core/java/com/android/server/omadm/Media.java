/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
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

/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2016 Verizon
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
 * See the License for the specific language governing permissions an
 * limitations under the License.
 */

package com.android.server.omadm;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Slog;

public class Media {

    private static final int FLAG_INTERNAL_DEVICE = 1;
    private static final int FLAG_EXTERNAL_DEVICE = 2;

    public static boolean DEBUG = true;
    private static final String TAG = Media.class.getSimpleName();

    private static final String[] PROJ_MEDIA_PATH_COLUMNS =
            new String[] {MediaStore.MediaColumns.DATA};

    private static String[] getMediaPaths(Context ctx, Uri contentUri){
        Cursor c = ctx.getContentResolver().query(contentUri,
                PROJ_MEDIA_PATH_COLUMNS, null, null, null);
        String[] result = null;
        int sizeArray = 0;
        if (c != null) {
            if (c.moveToFirst()) {
                sizeArray = c.getCount();
                result = new String[sizeArray];
                for (int idx = 0; idx < sizeArray; idx ++) {
                    c.moveToPosition(idx);
                    result[idx] = c.getString(0);
                    if (DEBUG) Slog.d(TAG, "getMediaPaths.item:" + result[idx]);
                }
            }
            c.close();
          }
        if (DEBUG) Slog.d(TAG, "getMediaPaths.size:" + sizeArray);
        return result;
    }

    private static String[] joinArrays(String[] arg0, String[] arg1) {
        String[] retVal;
        int arraySize = 0;
        if (arg0 != null) {
            arraySize += arg0.length;
        }
        if (arg1 != null) {
            arraySize += arg1.length;
        }
        retVal = new String[arraySize];
        int counter = 0;
        if (arg0 != null) {
            for (String arg0str: arg0) {
                retVal[counter++] = arg0str;
            }
        }
        if (arg1 != null) {
            for (String arg1str: arg1) {
                retVal[counter++] = arg1str;
            }
        }
        Slog.d(TAG, "arraySize = " + arraySize);
        return retVal;
    }

    public static String[] getDiskUsageVideo(Context ctx, int external) {
        String[] externals = null;
        String[] internals = null;
        if ((external & FLAG_EXTERNAL_DEVICE) != 0) {
            externals = getMediaPaths(ctx,
                    MediaStore.Video.Media.EXTERNAL_CONTENT_URI);
        }
        if ((external & FLAG_INTERNAL_DEVICE) != 0) {
            internals = getMediaPaths(ctx,
                    MediaStore.Video.Media.INTERNAL_CONTENT_URI);
        }
        return joinArrays(externals, internals);
    }

    public static String[] getDiskUsageAudio(Context ctx, int external) {
        String[] externals = null;
        String[] internals = null;
        if ((external & FLAG_EXTERNAL_DEVICE) != 0) {
            externals = getMediaPaths(ctx,
                    MediaStore.Audio.Media.EXTERNAL_CONTENT_URI);
        }
        if ((external & FLAG_INTERNAL_DEVICE) != 0) {
            internals = getMediaPaths(ctx,
                    MediaStore.Audio.Media.INTERNAL_CONTENT_URI);
        }
        return joinArrays(externals, internals);
    }

    public static String[] getDiskUsagePicts(Context ctx, int external) {
        String[] externals = null;
        String[] internals = null;
        if ((external & FLAG_EXTERNAL_DEVICE) != 0) {
            externals = getMediaPaths(ctx,
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        }
        if ((external & FLAG_INTERNAL_DEVICE) != 0) {
            internals = getMediaPaths(ctx,
                    MediaStore.Images.Media.INTERNAL_CONTENT_URI);
        }
        return joinArrays(externals, internals);
    }

}
