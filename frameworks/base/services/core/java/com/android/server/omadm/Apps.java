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

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageStatsObserver;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageStats;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.UserInfo;
import android.os.Environment;
import android.os.UserManager;
import android.os.storage.StorageManager;
import android.os.storage.VolumeInfo;
import android.util.Slog;

/**
 * @hide
 *
 */
public class Apps {

    private static final int FLAG_INTERNAL_DEVICE = 1;
    private static final int FLAG_EXTERNAL_DEVICE = 2;

    public static boolean DEBUG = true;
    private static final String TAG = Apps.class.getSimpleName();

    private Context mContext;

    /**
     * Gets list of paths of folders with applications
     * @param ctx instance of the Context
     * @return list of paths of folders with applications
     */
    public static List<String> getAppsPaths(Context ctx) {
        ArrayList<String> paths = new ArrayList<String>();
        List<PackageInfo> pkgs = ctx.getPackageManager()
                .getInstalledPackages(PackageManager.GET_UNINSTALLED_PACKAGES |
                        PackageManager.GET_DISABLED_COMPONENTS);
        for (PackageInfo info : pkgs){
            if (info.applicationInfo.dataDir != null) {
                paths.add(info.applicationInfo.dataDir);
                if (DEBUG) Slog.d(TAG, "getAppsPaths.data:" +
                        info.applicationInfo.dataDir);
            }
            if (info.applicationInfo.nativeLibraryDir != null) {
                paths.add(info.applicationInfo.nativeLibraryDir);
                if (DEBUG) Slog.d(TAG, "getAppsPaths.lib:" +
                        info.applicationInfo.nativeLibraryDir);
            }
            if (info.applicationInfo.sourceDir != null) {
                paths.add(info.applicationInfo.sourceDir);
                if (DEBUG) Slog.d(TAG, "getAppsPaths.source:" +
                        info.applicationInfo.sourceDir);
            }
        }
        if (DEBUG) Slog.d(TAG, "getAppsPaths.size:" + paths.size());
        return paths;
    }

    public static List<String> getCachePaths(Context ctx, int external) {
        ArrayList<String> paths = new ArrayList<String>();
        List<PackageInfo> pkgs = ctx.getPackageManager()
                .getInstalledPackages(PackageManager.GET_UNINSTALLED_PACKAGES |
                        PackageManager.GET_DISABLED_COMPONENTS);
        for (PackageInfo info : pkgs){
            try {
                Context packageContext = ctx.createPackageContext(
                        info.packageName, Context.CONTEXT_IGNORE_SECURITY);
                if ((external & FLAG_EXTERNAL_DEVICE) != 0) {
                    if (packageContext != null &&
                            packageContext.getExternalCacheDir() != null) {
                        paths.add(packageContext.getExternalCacheDir().getPath());
                        if (DEBUG) Slog.d(TAG, "getCachePaths.ext_cache:" +
                                packageContext.getExternalCacheDir().getPath());
                    }
                }
                if ((external & FLAG_INTERNAL_DEVICE) != 0) {
                    if (packageContext != null &&
                            packageContext.getCacheDir() != null) {
                        paths.add(packageContext.getCacheDir().getPath());
                        if (DEBUG) Slog.d(TAG, "getCachePaths.cache:" +
                                packageContext.getCacheDir().getPath());
                    }
                }
            } catch (NameNotFoundException excp) {
                /*
                 * Ignore this exception and continue getting information about available
                 * packages
                 */
                Slog.w(TAG, "getCachePaths.not_found.pkg:" + info.packageName);
            }
        }
        if (DEBUG) Slog.d(TAG, "getCachePaths.size:" + paths.size());
        return paths;
    }

    public static class MeasurementDetails {

        /**
         * Total cache disk usage by apps (over all users and profiles).
         */
        public long cacheSize;
        public long appsSize;

    }

    private static class StatsObserver extends IPackageStatsObserver.Stub {
        private final boolean mIsPrivate;
        private final MeasurementDetails mDetails;
        private final int mCurrentUser;
        public boolean mIsWaiting = false;

        private int mRemaining;

        public StatsObserver(boolean isPrivate, MeasurementDetails details, int currentUser,
                List<UserInfo> profiles, int remaining) {
            mIsPrivate = isPrivate;
            mDetails = details;
            mCurrentUser = currentUser;
            if (DEBUG) Slog.d(TAG, "StatsObserver(" + currentUser + ", " +
            remaining + ")");
            mRemaining = remaining;
        }

        @Override
        public void onGetStatsCompleted(PackageStats stats, boolean succeeded) {
            if (DEBUG) Slog.d(TAG, "onGetStatsCompleted(" +
                    stats.packageName + ", " + succeeded + "): nCBS = " +
                    mRemaining);
            synchronized (mDetails) {
                if (succeeded) {
                    addStatsLocked(stats);
                }
                if (--mRemaining == 0 && mIsWaiting) {
                    mDetails.notify();
                }
            }
        }

        public int getRemaining() {
            return mRemaining;
        }

        private void addStatsLocked(PackageStats stats) {
            if (mIsPrivate) {
                long codeSize = stats.codeSize;
                long dataSize = stats.dataSize;
                long cacheSize = stats.cacheSize;
                if (Environment.isExternalStorageEmulated()) {
                    // Include emulated storage when measuring internal. OBB is
                    // shared on emulated storage, so treat as code.
                    codeSize += stats.externalCodeSize + stats.externalObbSize;
                    dataSize += stats.externalDataSize + stats.externalMediaSize;
                    cacheSize += stats.externalCacheSize;
                }

                // Include cache for all users
                mDetails.cacheSize += cacheSize;
                mDetails.appsSize += dataSize + codeSize;

            } else {
                // Physical storage; only count external sizes
                mDetails.appsSize +=
                        stats.externalCodeSize + stats.externalDataSize +
                        stats.externalMediaSize + stats.externalObbSize;
                mDetails.cacheSize += stats.externalCacheSize;
            }
        }
    }

    private MeasurementDetails measureExactStorage() {
        final UserManager userManager = mContext.getSystemService(UserManager.class);
        final PackageManager packageManager = mContext.getPackageManager();

        final List<UserInfo> users = userManager.getUsers();
        final List<UserInfo> currentProfiles = userManager.getEnabledProfiles(
                ActivityManager.getCurrentUser());

        final MeasurementDetails details = new MeasurementDetails();

        if (mVolume == null || !mVolume.isMountedReadable()) {
            Slog.w(TAG, "mVolume is not accessable");
            return details;
        }

        // Measure all apps hosted on this volume for all users
        if (mVolume.getType() == VolumeInfo.TYPE_PRIVATE) {
            if (DEBUG) Slog.d(TAG, "mVolume.getType() == VolumeInfo.TYPE_PRIVATE");
            final List<ApplicationInfo> apps = packageManager.getInstalledApplications(
                    PackageManager.GET_UNINSTALLED_PACKAGES
                    | PackageManager.GET_DISABLED_COMPONENTS);

            final List<ApplicationInfo> volumeApps = new ArrayList<>();
            for (ApplicationInfo app : apps) {
                if (Objects.equals(app.volumeUuid, mVolume.getFsUuid())) {
                    volumeApps.add(app);
                }
            }

            final int count = users.size() * volumeApps.size();
            if (count == 0) {
                if (DEBUG) Slog.d(TAG, "users.size() = " + users.size() +
                        ", volumeApps.size() = " + volumeApps.size());
                return details;
            }

            final StatsObserver observer = new StatsObserver(true, details,
                    ActivityManager.getCurrentUser(), currentProfiles, count);
            for (UserInfo user : users) {
                for (ApplicationInfo app : volumeApps) {
                    packageManager.getPackageSizeInfo(app.packageName,  observer);
                }
            }
            synchronized (details) {
                int numCBs = observer.getRemaining();
                if (numCBs > 0) {
                    observer.mIsWaiting = true;
                    if (DEBUG) Slog.d(TAG, "if (numCBs = " +
                            observer.getRemaining() + " > 0) {wait}");
                    try{
                        details.wait(numCBs * 50);
                    } catch (InterruptedException excp) {
                        Slog.w(TAG,"measureExactStorage():InterruptedException",
                                excp);
                    }
                    observer.mIsWaiting = false;
                }
            }
        } else {
            if (DEBUG) Slog.d(TAG, "mVolume.getType() != VolumeInfo.TYPE_PRIVATE");
        }
        return details;
    }

    private StorageManager mStorageManager;
    private UserManager mUserManager;

    private String mVolumeId;
    private VolumeInfo mVolume;

    public Apps(Context ctx) {
        mContext = ctx;
        mUserManager = mContext.getSystemService(UserManager.class);
        mStorageManager = mContext.getSystemService(StorageManager.class);

        mVolumeId = VolumeInfo.ID_PRIVATE_INTERNAL;
        mVolume = mStorageManager.findVolumeById(mVolumeId);

    }

    public static final int TYPE_APPS = 0;
    public static final int TYPE_CACHE = 1;

    /**
     * Returns disk usage space by applications or cache
     * @param path mount point of the volume
     * @param type : 0 - apps, 1 - cache
     */
    public long getDiskUsage(int type) {
        long retVal = 0L;
        MeasurementDetails details = measureExactStorage();
        switch (type) {
            case TYPE_APPS:
                retVal = details.appsSize;
                break;
            case TYPE_CACHE:
                retVal = details.cacheSize;
                break;

            default:
                break;
        }
        if (DEBUG) Slog.d(TAG, "getAppsBytes():" + retVal);
        return retVal;
    }

}
