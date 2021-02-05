/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef _FUMO_ERROR_H_
#define _FUMO_ERROR_H_

/**
 *  @brief FUMO Result and status codes according to
 *         [2015-12-02] Verizon_OTADM_Requirements_Ref_Client - Dec02
 *         1.2.1.3 RESULT AND STATUS CODES
 */
/** @brief Successful Replace on PkgURL */
#define FUMO_RC_SUCCESSFUL_UPDATE                   200
/** @brief Successful EXEC on downloadAndUpdate */
#define FUMO_RC_ACCEPTED_FOR_PROCESSING             202
/** @brief Generic Alert Message with Alert Type 1226 */
#define FUMO_RC_SUCCESSFUL_DOWNLOAD                 255
/** @brief  Corrupted firmware update package, did not store correctly.
 *          Detected, for example, by mismatched CRCs between actual
 *          and expected.
 */
#define FUMO_RC_UPDATE_FAILED                       402
/** @brief <Item> tag with the message "KEY_DEFERRED" "WIFI_DEFERRED"
 *              "LOW_BATTERY" "DOWNLOAD_DEFERRED" "UPDATE_DEFERRED"
 *              "LOW_MEMORY"
 */
#define FUMO_RC_UPDATE_DEFERRED                     403
/** @brief  Not Found - The specified data item doesn't exist on the recipient.
 *          This may also imply that the stated URI for the location of the new
 *          management object cannot be resolved. Update Failed - Failure to
 *          positively validate digital signature of update package
 */
#define FUMO_RC_NOT_FOUND_OR_UPDATE_FAILED          404
/** @brief Firmware Update Package is Not Acceptable */
#define FUMO_RC_UPDATE_PACKAGE_NOT_ACCEPTABLE       405
/** @brief  The Alternate Download Server is Unavailable or Does not Respond*/
#define FUMO_RC_DL_SERVER_IS_UNAVAILABLE            412
/** @brief Firmware download fails due to device is out of memory */
#define FUMO_RC_DOWNLOAD_FAILED_NO_MEM              501
/** @brief Firmware update fails due to device is out of memory */
#define FUMO_RC_UPDATE_FAILED_NO_MEM                502
/** @brief  SU Cancel Success no Firmware to terminate
 *          Generic Alert Message with Alert Type 1226
 */
#define FUMO_RC_SU_CANCEL_SUCCESS_NO_FIRMWARE       552
/** @brief  SU Cancel Success during Deferred Download
 *          Generic Alert Message with Alert Type 1226
 */
#define FUMO_RC_SU_CANCEL_SUCCESS_DEFERRED_DOWNLOAD 553
/** @brief  SU Cancel Success prior to Update
 *          Generic Alert Message with Alert Type 1226
 */
#define FUMO_RC_SU_CANCEL_SUCCESS_UPDATE            554
/** @brief  Result code on Check Status screen for a Download that timed out
 *          due to WifiOnlyTimer expirey.
 */
#define FUMO_RC_DOWNLOAD_FAILED_TIME_OUT            555
/** @brief  Result code if Enterprise Policy reject download
 */
#define FUMO_ENTERPRISE_POLICY_REJECT               599
/** @brief Session Abort Alert Code */
#define FUMO_RC_SESSION ABORT                       1223

/** @brief Extended code for 403 status*/
#define FUMO_RC_PRV_DEFERRED_MIN                    1600
/** @brief Internal Code  for 403 status, <Item> tag with the message
 *         "DOWNLOAD_DEFERRED" will be added
 */
#define FUMO_RC_PRV_DOWNLOAD_DEFERRED               1601
/** @brief Internal Code for 403 status, <Item> tag with the message
 *         "UPDATE_DEFERRED" will be added
 */
#define FUMO_RC_PRV_UPDATE_DEFERRED                 1602
/** @brief Internal Code for 403 status, <Item> tag with the message
 *         "UPDATE_DEFERRED" and <Item> tag with the message "DOWNLOAD_DEFERRED"
 *         will be added
 */
#define FUMO_RC_PRV_DOWNLOAD_UPDATE_DEFERRED        1603
/** @brief Internal Code for 403 status, <Item> tag with the message
 *        "KEY_DEFERRED" will be added
 */
#define FUMO_RC_PRV_UPDATE_DEFERRED_KEY_DEFERRED    1604
/** @brief Internal Code for 403 status, <Item> tag with the message
 *         "KEY_DEFERRED" will be added
 */
#define FUMO_RC_PRV_UPDATE_DEFERRED_WIFI_DEFERRED   1605
/** @brief Internal Code for 403 status, <Item> tag with the message
 *         "LOW_BATTERY" will be added
 */
#define FUMO_RC_PRV_UPDATE_DEFERRED_LOW_BATTERY     1606
/** @brief Internal Code for 403 status, <Item> tag with the message
 *         "LOW_MEMORY" will be added
 */
#define FUMO_RC_PRV_UPDATE_DEFERRED_LOW_MEMORY      1607

#define FUMO_RC_PRV_DEFERRED_MAX                    1699

#endif /* _FUMO_ERROR_H_ */
