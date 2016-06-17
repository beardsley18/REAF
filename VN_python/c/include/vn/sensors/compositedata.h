/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#ifndef _VNCOMPOSITEDATA_H_
#define _VNCOMPOSITEDATA_H_

#include "vn/int.h"
#include "vn/math.h"
#include "vn/math/vector.h"
#include "vn/bool.h"

#ifdef __cplusplus
extern "C" {
#endif


/*Forward Declarations*/
typedef enum AttitudeGroup AttitudeGroup;
typedef enum CommonGroup CommonGroup;
typedef enum GpsGroup GpsGroup;
typedef enum ImuGroup ImuGroup;
typedef enum InsGroup InsGroup;
typedef enum TimeGroup TimeGroup;

typedef struct InGroup InGroup;
typedef struct VnCriticalSection VnCriticalSection;
typedef struct VnUartPacket VnUartPacket;

/** \brief Enumeration of the velocity types */
typedef enum VelocityType
{
	CDVEL_None,
	CDVEL_GpsNed,
	CDVEL_GpsEcef,
	CDVEL_EstimatedNed,
	CDVEL_EstimatedEcef,
	CDVEL_EstimatedBody
} VelocityType;

/** \brief Composite structure of all available data types from VectorNav sensors. */
typedef struct VnCompositeData
{
	vec3f yawPitchRoll;					/**< Yaw, pitch, roll data. */
	vec4f quaternion;					/**< Quaternion data. */
	mat3f directionCosineMatrix;		/**< Direction cosine matrix data. */
	vec3d positionGpsLla;				/**< GPS latitude, longitude, altitude data. */
	vec3d positionGpsEcef;				/**< GPS earth-centered, earth-fixed data. */
	vec3d positionEstimatedLla;			/**< Estimated latitude, longitude, altitude data. */
	vec3d positionEstimatedEcef;		/**< Estimated earth-centered, earth-fixed position data. */
	enum VelocityType velocityType;     /**< Type of velocity in the struct. */
	vec3f velocityGpsNed;				/**< GPS velocity NED data. */
	vec3f velocityGpsEcef;				/**< GPS velocity ECEF data. */
	vec3f velocityEstimatedBody;		/**< Estimated velocity body data. */
	vec3f velocityEstimatedNed;			/**< Estimated velocity NED data. */
	vec3f velocityEstimatedEcef;		/**< Estimated velocity ECEF data. */
	vec3f magnetic;						/**< Magnetic data. */
	vec3f magneticUncompensated;		/**< Magnetic uncompensated data. */
	vec3f magneticNed;					/**< Magnetic NED data. */
	vec3f magneticEcef;					/**< Magnetic ECEF data. */
	#if EXTRA
	vec3f magneticRaw;					/**< Magnetic raw data. */
	#endif
	vec3f acceleration;					/**< Acceleration data. */
	vec3f accelerationUncompensated;	/**< Acceleration uncompensated data. */
	vec3f accelerationNed;				/**< Acceleration NED data. */
	vec3f accelerationEcef;				/**< Acceleration ECEF data. */
	vec3f accelerationLinearBody;		/**< Acceleration linear body data. */
	vec3f accelerationLinearNed;		/**< Acceleration linear NED data. */
	vec3f accelerationLinearEcef;		/**< Acceleration linear ECEF data. */
	#if EXTRA
	vec3f accelerationRaw;				/**< Acceleration raw data. */
	#endif
	vec3f angularRate;					/**< Angular rate data. */
	vec3f angularRateUncompensated;		/**< Angular rate uncompensated data. */
	#if EXTRA
	vec3f angularRateRaw;				/**< Angular rate raw data. */
	#endif
	float temperature;					/**< Temperature data. */
	#if EXTRA
	float temperatureRaw;				/**< Temperature raw data. */
	#endif
	float pressure;						/**< Pressure data. */
	uint64_t timeStartup;				/**< Time startup data. */
	float deltaTime;					/**< Delta time data. */
	vec3f deltaTheta;					/**< Delta theta data. */
	vec3f deltaVelocity;				/**< Delta velocity data. */
	double tow;							/**< GPS time of week data. */
	uint16_t week;						/**< Week data. */
	uint8_t gpsFix;						/**< GPS fix data. */
	uint8_t numSats;					/**< NumSats data. */
	uint64_t timeGps;					/**< TimeGps data. */
	uint64_t timeGpsPps;				/**< TimeGpsPps data. */
	uint64_t gpsTow;					/**< GpsTow data. */
	vec3f attitudeUncertainty;			/**< Attitude uncertainty data. */
	vec3f positionUncertaintyGpsNed;	/**< GPS position uncertainty NED data. */
	vec3f positionUncertaintyGpsEcef;	/**< GPS position uncertainty ECEF data. */
	float positionUncertaintyEstimated;	/**< Estimated position uncertainty data. */
	float velocityUncertaintyGps;		/**< GPS velocity uncertainty data. */
	float velocityUncertaintyEstimated;	/**< Estimated velocity uncertainty data. */
	uint32_t timeUncertainty;			/**< Time uncertainty data. */
	uint16_t vpeStatus;					/**< VpeStatus data. */
	uint16_t insStatus;					/**< InsStatus data. */
	uint64_t timeSyncIn;				/**< TimeSyncIn data. */
	uint32_t syncInCnt;					/**< SyncInCnt data. */
	uint16_t sensSat;					/**< SensSat data. */
	#if EXTRA
	vec3f yprRates;						/**< YprRates data. */
	#endif

} VnCompositeData;

/** \brief Indicates if course over ground has valid data
*
* \param[in] compositeData The associated VnCompositeData structure.
* \return Flag indicating if the course over ground data is available. */
bool VnCompositeData_hasCourseOverGround(VnCompositeData* compositeData);

/** \brief Computers the course over ground from any velocity data available
*
* \param[in] compositeData The associated VnCompositeData structure.
* \param[out] courseOverGroundOut The computered course over ground.
* \return Flag indicating if the calculation was successful. */
bool VnCompositeData_courseOverGround(VnCompositeData* compositeData, float* courseOverGroundOut);

/** \brief Indicates if speed over ground has valid data..
*
* \param[in] compositeData The associated VnCompositeData structure.
* \return Flag indicating if the speed over ground data is available. */
bool VnCompositeData_hasSpeedOverGround(VnCompositeData* compositeData);

/** \brief Computers the speed over ground from any velocity data available
*
* \param[in] compositeData The associated VnCompositeData structure.
* \param[out] speedOverGroundOut The computered course over ground.
* \return Flag indicating if the calculation was successful. */
bool VnCompositeData_speedOverGround(VnCompositeData* compositeData, float* speedOverGroundOut);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processBinaryPacket(VnCompositeData* compositeData, VnUartPacket* packet, VnCriticalSection* criticalSection);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processAsciiAsyncPacket(VnCompositeData* compositeData, VnUartPacket* packet, VnCriticalSection* criticalSection);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processBinaryPacketCommonGroup(VnCompositeData* compositeData, VnUartPacket* packet, CommonGroup groupFlags);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processBinaryPacketTimeGroup(VnCompositeData* compositeData,  VnUartPacket* packet, TimeGroup groupFlags);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processBinaryPacketImuGroup(VnCompositeData* compositeData,  VnUartPacket* packet, ImuGroup groupFlags);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processBinaryPacketGpsGroup(VnCompositeData* compositeData,  VnUartPacket* packet, GpsGroup groupFlags);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processBinaryPacketAttitudeGroup(VnCompositeData* compositeData,  VnUartPacket* packet, AttitudeGroup groupFlags);

/** \brief
*
* \param[in]
* \param[out]
* \return
*/
void VnCompositeData_processBinaryPacketInsGroup(VnCompositeData* compositeData,  VnUartPacket* packet, InsGroup groupFlags);

#ifdef __cplusplus
}
#endif

#endif
