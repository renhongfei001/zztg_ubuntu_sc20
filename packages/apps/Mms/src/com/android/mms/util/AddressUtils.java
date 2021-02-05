/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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
package com.android.mms.util;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Mms.Addr;
import android.text.TextUtils;
import android.os.SystemProperties;

import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.mms.MmsApp;
import com.android.mms.R;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.PduPersister;

public class AddressUtils {
    private static PhoneNumberUtil mPhoneNumberUtil;

    private static boolean mOnCertificationMode = SystemProperties.getBoolean("persist.certification.mode", false);
    private static String mBaseBandVersion = SystemProperties.get("gsm.version.baseband", "null");

    private AddressUtils() {
        // Forbidden being instantiated.
    }

    public static String getFrom(Context context, Uri uri) {
        String msgId = uri.getLastPathSegment();
        Uri.Builder builder = Mms.CONTENT_URI.buildUpon();

        builder.appendPath(msgId).appendPath("addr");

        Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(),
                            builder.build(), new String[] {Addr.ADDRESS, Addr.CHARSET},
                            Addr.TYPE + "=" + PduHeaders.FROM, null, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    String from = cursor.getString(0);

                    if (!TextUtils.isEmpty(from)) {
                        byte[] bytes = PduPersister.getBytes(from);
                        int charset = cursor.getInt(1);
                        return new EncodedStringValue(charset, bytes)
                                .getString();
                    }
                }
            } finally {
                cursor.close();
            }
        }
        return context.getString(R.string.hidden_sender_address);
    }

    /**
     * isPossiblePhoneNumberCanDoFileAccess does a more accurate test if the input is a
     * phone number, but it can do file access to load country prefixes and other info, so
     * it's not safe to call from the UI thread.
     * @param query the phone number to test
     * @return true if query looks like a valid phone number
     */
    public static boolean isPossiblePhoneNumberCanDoFileAccess(String query) {
        String currentCountry = MmsApp.getApplication().getCurrentCountryIso().toUpperCase();
        if (mPhoneNumberUtil == null) {
            mPhoneNumberUtil = PhoneNumberUtil.getInstance();
        }
        return mPhoneNumberUtil.isPossibleNumber(query, currentCountry);
    }

    //andrew.hu add start
    public static String TransferFormat(String title,int loglevel){
    if (mOnCertificationMode){
        if (mBaseBandVersion.contains("SC20A")){
        String trimStr = title.replaceAll(" ","");
        if(loglevel == 1){
            android.util.Log.i("andrew","trimStr_from="+trimStr);
        }else if(loglevel == 2){
            android.util.Log.i("andrew","trimStr_title="+trimStr);
        }else if(loglevel == 3){
            android.util.Log.i("andrew","trimStr_number="+trimStr);
        }
        if(trimStr.length() > 11 && trimStr.contains(",")){
            String[] numstr = trimStr.split(",");
            String newStr = "";
            for (int i = 0; i < numstr.length; i++) {
                if(numstr[i].matches("[0-9]{1,}") || (numstr[i].contains("+")) || (numstr[i].contains("-"))){
                    if(!numstr[i].contains("@") && !numstr[i].contains(",")){
                        String newSub = SubStrf(numstr[i]);
                        if(newSub.contains("+1")){
                           }else{
                        if(newSub.contains("0111")){
                            numstr[i] = numstr[i].replace("011", "+");
                        }else{
                           if(newSub.contains("+")){
                              numstr[i] = numstr[i].replace("+", "011");
                           }else{
                              //<IDD><Country Code><number>
                              //<Country Code><Number>
                           if(newSub.contains("011") || newSub.contains("001")){
                            }else{
                               numstr[i] = "011" + numstr[i];
                            }
                         }
                       }
                   }
                }
              }
              newStr = newStr + numstr[i] + ", ";
            }
          title = newStr;
        }else if(trimStr.matches("[0-9]{1,}") || (trimStr.contains("+")) || (trimStr.contains("-"))){
            if(!trimStr.contains("@") && !trimStr.contains(",")){
            String newSub = SubStrf(trimStr);
            if(newSub.contains("+1")){

            }else{
                if(newSub.contains("0111")){
                    title = title.replace("011", "+");
                }else{
                    if(newSub.contains("+")){
                        title = title.replace("+", "011");
                    }else{
                    //<IDD><Country Code><number>
                    //<Country Code><Number>
                    if(newSub.contains("011") || newSub.contains("001")){

                    }else{
                        title = "011" + title;
                    }
                    }
                }
             }
          }
        }
        }
    }
            return title;
    }
    //andrew.hu add SubStrf
    private static String SubStrf(String num){
        if(num != null && num.length() >= 4){
            String newSub = num.substring(0, 4);
            return newSub;
        }
        return num;
    }
}
