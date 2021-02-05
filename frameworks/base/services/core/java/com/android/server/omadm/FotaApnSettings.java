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
 * Copyright (C) 2014 The Android Open Source Project
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

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.NetworkUtils;
import android.net.Uri;
import android.provider.Telephony;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;
import com.android.server.omadm.FotaNetworkException;

import java.net.URI;
import java.net.URISyntaxException;
import android.util.Log;

/**
 * APN settings used for MMS transactions
 */
public class FotaApnSettings {
    // URL
    private final String mServer;
    // proxy address
    private final String mProxyAddress;
    // proxy port
    private final int mProxyPort;
    // Debug text for this APN: a concatenation of interesting columns of this APN
    private final String mDebugText;

    //
    private static final String mAdminApnType = PhoneConstants.APN_TYPE_FOTA;

    private static final String[] APN_PROJECTION = {
            Telephony.Carriers.TYPE,
            Telephony.Carriers.MMSC,
            Telephony.Carriers.MMSPROXY,
            Telephony.Carriers.MMSPORT,
            Telephony.Carriers.NAME,
            Telephony.Carriers.APN,
            Telephony.Carriers.BEARER_BITMASK,
            Telephony.Carriers.PROTOCOL,
            Telephony.Carriers.ROAMING_PROTOCOL,
            Telephony.Carriers.AUTH_TYPE,
            Telephony.Carriers.MVNO_TYPE,
            Telephony.Carriers.MVNO_MATCH_DATA,
            Telephony.Carriers.PROXY,
            Telephony.Carriers.PORT,
            Telephony.Carriers.SERVER,
            Telephony.Carriers.USER,
            Telephony.Carriers.PASSWORD,
    };
    private static final int COLUMN_TYPE         = 0;
    private static final int COLUMN_MMSC         = 1;
    private static final int COLUMN_MMSPROXY     = 2;
    private static final int COLUMN_MMSPORT      = 3;
    private static final int COLUMN_NAME         = 4;
    private static final int COLUMN_APN          = 5;
    private static final int COLUMN_BEARER       = 6;
    private static final int COLUMN_PROTOCOL     = 7;
    private static final int COLUMN_ROAMING_PROTOCOL = 8;
    private static final int COLUMN_AUTH_TYPE    = 9;
    private static final int COLUMN_MVNO_TYPE    = 10;
    private static final int COLUMN_MVNO_MATCH_DATA = 11;
    private static final int COLUMN_PROXY        = 12;
    private static final int COLUMN_PORT         = 13;
    private static final int COLUMN_SERVER       = 14;
    private static final int COLUMN_USER         = 15;
    private static final int COLUMN_PASSWORD     = 16;


    /**
     * Load APN settings from system
     * @param context
     * @param apnName the optional APN name to match
     * @param the caller ID for logging
     * @param check CURRENT
     */
    public static FotaApnSettings load(Context context, String apnName, int subId, String logTag, boolean chCurr)
            throws FotaNetworkException {
        Log.i(logTag, "Loading APN using name " + apnName);

        final StringBuilder selectionBuilder = new StringBuilder();
        String[] selectionArgs = null;

        if (chCurr) {
            selectionBuilder.append(Telephony.Carriers.CURRENT).append(" IS NOT NULL");
        }
        apnName = trimWithNullCheck(apnName);
        if (!TextUtils.isEmpty(apnName)) {
            if (selectionBuilder.length() > 0) {
                selectionBuilder.append(" AND ");
            }
                selectionBuilder.append(Telephony.Carriers.APN).append("=?");
                selectionArgs = new String[] { apnName };
        }

        Cursor cursor = null;
        try {
            cursor = SqliteWrapper.query(
                    context,
                    context.getContentResolver(),
                    Uri.withAppendedPath(Telephony.Carriers.CONTENT_URI, "/subId/" + subId),
                    APN_PROJECTION,
                    selectionBuilder.toString(),
                    selectionArgs,
                    null/*sortOrder*/);
            if (cursor != null) {
                String serverUrl = null;
                String proxyAddress = null;
                // Default proxy port to 80
                int proxyPort = 80;
                while (cursor.moveToNext()) {
                    // Read values from APN settings
                    if (isValidApnType(cursor.getString(COLUMN_TYPE))) {
                        serverUrl = trimWithNullCheck(cursor.getString(COLUMN_SERVER));
                        if (!TextUtils.isEmpty(serverUrl)) {
                            serverUrl = NetworkUtils.trimV4AddrZeros(serverUrl);
                            try {
                                new URI(serverUrl);
                            } catch (URISyntaxException e) {
                                throw new FotaNetworkException("Invalid Server url " + serverUrl);
                            }
                        }
                        proxyAddress = trimWithNullCheck(cursor.getString(COLUMN_PROXY));
                        if (!TextUtils.isEmpty(proxyAddress)) {
                            proxyAddress = NetworkUtils.trimV4AddrZeros(proxyAddress);
                            final String portString =
                                    trimWithNullCheck(cursor.getString(COLUMN_PORT));
                            if (!TextUtils.isEmpty(portString)) {
                                try {
                                    proxyPort = Integer.parseInt(portString);
                                } catch (NumberFormatException e) {
                                    Log.e(logTag, "Invalid port " + portString + ", use 80");
                                }
                            }
                        }
                        return new FotaApnSettings(
                                serverUrl, proxyAddress, proxyPort, getDebugText(cursor));
                    }
                }

            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        throw new FotaNetworkException("Can not find valid APN");
    }

    private static String getDebugText(Cursor cursor) {
        final StringBuilder sb = new StringBuilder();
        sb.append("APN [");
        for (int i = 0; i < cursor.getColumnCount(); i++) {
            final String name = cursor.getColumnName(i);
            final String value = cursor.getString(i);
            if (TextUtils.isEmpty(value)) {
                continue;
            }
            if (i > 0) {
                sb.append(' ');
            }
            sb.append(name).append('=').append(value);
        }
        sb.append("]");
        return sb.toString();
    }

    private static String trimWithNullCheck(String value) {
        return value != null ? value.trim() : null;
    }

    public FotaApnSettings(String serverUrl, String proxyAddr, int proxyPort, String debugText) {
        mServer = serverUrl;
        mProxyAddress = proxyAddr;
        mProxyPort = proxyPort;
        mDebugText = debugText;
   }

    public String getserverUrl() {
        return mServer;
    }

    public String getProxyAddress() {
        return mProxyAddress;
    }

    public int getProxyPort() {
        return mProxyPort;
    }

    public boolean isProxySet() {
        return !TextUtils.isEmpty(mProxyAddress);
    }

    private static boolean isValidApnType(String types) {
        if (TextUtils.isEmpty(types)) {
            return false;
        }
        for (String type : types.split(",")) {
            type = type.trim();
            if (type.equals(mAdminApnType)) {
                return true;
            }
        }
        return false;
    }

    public String toString() {
        return mDebugText;
    }
}
