/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef SCM_ERROR_H
#define SCM_ERROR_H

/**
 * @brief  The SCOMO  Result Code of the operation.
 *         The Result Code MUST be one of the values defined below
 *         according to Open Mobile Alliance
 *         OMA-TS-DM-SCOMO-V1_0-20081024-C  8.7
 */
/** @brief The Request Accepted for processing”) */
#define  SCM_ACCEPTED_FOR_PROCESSING                     202

/** @brief The Request has Succeeded */
#define  SCM_REQUEST_SUCCESSFULL                         1200

/** @brief  1250-1299 Successful – Vendor Specified Successful Operation
 * with vendor specified Result
 */
#define  SCM_SUCCESSFUL_VENDOR_01                        1250
#define  SCM_SUCCESSFUL_VENDOR_50                        1299

/** @brief Client error – based on User or Device behaviour */
#define  SCM_CLIENT_ERROR                                1400

/** @brief User chose not to accept the operation when prompted */
#define  SCM_USER_CANCELLED                              1401


/** @brief The Software Component download failed */
#define  SCM_DOWNLOAD_FAILED                             1402

/** @brief Authentication was Required but Authentication Failure was
 *         encountered when downloading Software Component
 */
#define  SCM_AD_AUTH_FAILED                              1403

/**
 * @brief The download failed due to insufficient memory in the
 *        Device to save the Delivery Package.
 */
#define  SCM_DL_FAILED_OUT_OF_MEMORY                     1404

/**
 * @brief  Software Component installation failed in the Device
 */
#define  SCM_INSTALL_FAILED                              1405

/**
 * @brief The install failed because there wasn’t sufficient memory to
 *        install the Software Component in the Device.
 */
#define  SCM_INSTALL_FAILED_OUT_OF_MEMORY                1406

/** @brief  Failure to positively validate digital signature of the Delivery
 *          Package
 */
#define  SCM_FAILED_PACKAGE_VALIDATION                   1407

/** @brief The Software Component Remove operation failed */
#define  SCM_REMOVE_FAILED                               1408

/**  @brief The Software Component Activate operation failed */
#define  SCM_ACTIVATE_FAILED                             1409

/**  @brief The Software Component Deactivate operation failed */
#define  SCM_DEACTIVATE_FAILED                           1410

/**  @brief The Device does not support the requested operation */
#define  SCM_NOT_IMPLEMENTED                             1411

/**  @brief Indicates failure not defined by any other error code */
#define  SCM_UNDEFINED_ERROR                             1412

/**
 * @brief The Operation has been rejected because the device does not
 *          support the target environment type.
 */
#define  SCM_OPERATION_REJECTED                          1413

/**
 * @brief 1450-1499 Vendor Specified Client Error encountered for Operation
 *        with vendor specified Result Code
 */
#define  SCM_VENDOR_CLIENT_ERROR_01                      1450
#define  SCM_VENDOR_CLIENT_ERROR_50                      1499

/** @brief  Alternate Download Server Error Encountered */
#define  SCM_DL_SERVER_ERROR                             1500

/** @brief  The Alternate Download Server is unavailable or does not respond */
#define  SCM_DL_SERVER_UNAVAILABLE                       1501

/**  @brief 1550-1599 Alternate Download Server Error encountered for Operation
 *          with vendor specified Result Code
 */
#define  SCM_DL_SERVER_ERROR_VENDOR_SPECIFIC_01          1550
#define  SCM_DL_SERVER_ERROR_VENDOR_SPECIFIC_50          1599

#endif // SCM_ERROR_H
