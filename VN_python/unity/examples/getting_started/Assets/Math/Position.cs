using System;
using System.Collections.Generic;
using System.Text;

namespace VectorNav.Math
{

/// <summary>
/// Representation of a position/location.
/// </summary>
public struct PositionD
{
	#region Types

	private enum PositionType
	{
		Lla,
		Ecef
	}

	#endregion

	#region Constructors

	private PositionD(PositionType type, object position)
	{
		_underlyingType = type;
		_positionData = position;
	}

	#endregion

	#region Methods

	/// <summary>
	/// Creates a new <c>PositionD</c> from a latitude, longitude, altitude.
	/// </summary>
	/// <param name="lla">
	/// The position expressed as a latitude, longitude, altitude.
	/// </param>
	/// <returns>
	/// The new <c>PositionD</c>.
	/// </returns>
	public static PositionD FromLla(vec3d lla)
	{
		return new PositionD(PositionType.Lla, lla);
	}

	/// <summary>
	/// Creates a new <c>PositionD</c> from an earth-centered, earth-fixed.
	/// </summary>
	/// <param name="ecef">
	/// The position expressed as an earth-centered, earth-fixed.
	/// </param>
	/// <returns>
	/// The new <c>PositionD</c>.
	/// </returns>
	public static PositionD FromEcef(vec3d ecef)
	{
		return new PositionD(PositionType.Ecef, ecef);
	}

	#endregion

	private readonly PositionType _underlyingType;
	private readonly object _positionData;
}

}

#if DISABLED
/// <summary>
/// Representation of an orientation/attitude.
/// </summary>
public struct AttitudeF
{


	#region Properties


	/// <summary>
	/// Returns the orientation as represented in yaw, pitch, roll in degrees.
	/// </summary>
	public vec3f YprInDegs
	{
		get
		{
			switch (_underlyingType)
			{
				case AttitudeType.YprDegs:
					return (vec3f) _attitudeData;
				case AttitudeType.YprRads:
					return Conv.Rad2Deg((vec3f) _attitudeData);
				case AttitudeType.Quat:
					return Conv.Quat2YprInDegs((vec4f) _attitudeData);
				case AttitudeType.Dcm:
					return Conv.Dcm2YprInDegs((mat3f) _attitudeData);
				default:
					// Don't expect to ever get here.
					throw new NotImplementedException();
			}
		}
	}

	/// <summary>
	/// Returns the orientation as represented in yaw, pitch, roll in radians.
	/// </summary>
	public vec3f YprInRads
	{
		get
		{
			switch (_underlyingType)
			{
				case AttitudeType.YprRads:
					return (vec3f) _attitudeData;
				case AttitudeType.YprDegs:
					return Conv.Deg2Rad((vec3f) _attitudeData);
				case AttitudeType.Quat:
					return Conv.Quat2YprInRads((vec4f) _attitudeData);
				case AttitudeType.Dcm:
					return Conv.Dcm2YprInRads((mat3f) _attitudeData);
				default:
					// Don't expect to ever get here.
					throw new NotImplementedException();
			}
		}
	}

	/// <summary>
	/// Returns the orientation as represented in quaternion.
	/// </summary>
	public vec4f Quat
	{
		get
		{
			switch (_underlyingType)
			{
				case AttitudeType.Quat:
					return (vec4f) _attitudeData;
				case AttitudeType.YprDegs:
					return Conv.YprInDegs2Quat((vec3f) _attitudeData);
				case AttitudeType.YprRads:
					return Conv.YprInRads2Quat((vec3f) _attitudeData);
				case AttitudeType.Dcm:
					return Conv.Dcm2Quat((mat3f) _attitudeData);
				default:
					// Don't expect to ever get here.
					throw new NotImplementedException();
			}
		}
	}

	/// <summary>
	/// Returns the orientation as represented by a direction cosine matrix.
	/// </summary>
	public mat3f Dcm
	{
		get
		{
			switch (_underlyingType)
			{
				case AttitudeType.Dcm:
					return (mat3f) _attitudeData;
				case AttitudeType.YprDegs:
					return Conv.YprInDegs2Dcm((vec3f) _attitudeData);
				case AttitudeType.YprRads:
					return Conv.YprInRads2Dcm((vec3f) _attitudeData);
				case AttitudeType.Quat:
					return Conv.Quat2Dcm((vec4f) _attitudeData);
				default:
					// Don't expect to ever get here.
					throw new NotImplementedException();
			}
		}
	}

	#endregion

	

	

	
}
#endif