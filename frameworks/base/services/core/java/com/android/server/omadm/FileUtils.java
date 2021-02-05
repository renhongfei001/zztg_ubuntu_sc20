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

package com.android.server.omadm;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.android.server.OmadmService;

import android.os.ParcelFileDescriptor;
import android.util.Slog;

/**
 * @hide
 */
public class FileUtils {

    private static final String TAG = OmadmService.class.getSimpleName() +
            "." + FileUtils.class.getSimpleName();

    private static final String[] ALLOWED_PATHS = new String[] {
        "/data/vendor/verizon/dmclient/data/updateInfo.json",
        "/data/vendor/verizon/dmclient/data/last_update_firmware_version"
    };

    public static boolean isPathAllowed(String path) {
        for (String allowedPath: ALLOWED_PATHS) {
            if (allowedPath.equals(path)) {
                return true;
            }
        }
        return false;
    }

    public static ParcelFileDescriptor pipeTo(InputStream inputStream)
            throws IOException {
        ParcelFileDescriptor[] pipe = ParcelFileDescriptor.createPipe();
        ParcelFileDescriptor readSide = pipe[0];
        ParcelFileDescriptor writeSide = pipe[1];

        new TransferThread(
                new ParcelFileDescriptor.AutoCloseOutputStream(writeSide),
                inputStream).start();

        return readSide;
    }

    static class TransferThread extends Thread {
        final InputStream mIn;
        final OutputStream mOut;
        private static final int BUFF_SIZE = 1024;

        TransferThread(OutputStream out, InputStream in) {
            super("ParcelFileDescriptor Transfer Thread");
            mIn = in;
            mOut = out;
            setDaemon(true);
        }

        @Override
        public void run() {
            byte[] buf = new byte[BUFF_SIZE];
            int len;

            try {
                while ((len = mIn.read(buf)) > 0) {
                    mOut.write(buf, 0, len);
                }
                mOut.flush(); // just to be safe
            } catch (IOException e) {
                Slog.e(TAG, "TransferThread", e);
            }
            finally {
                try {
                    mIn.close();
                } catch (IOException e) {
                }
                try {
                    mOut.close();
                } catch (IOException e) {
                }
            }
        }
    }
}
