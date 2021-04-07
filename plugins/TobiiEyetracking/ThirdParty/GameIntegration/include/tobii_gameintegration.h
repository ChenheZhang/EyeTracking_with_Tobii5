#pragma once

#define TGI_VERSION_MAJOR 4
#define TGI_VERSION_MINOR 1
#define TGI_VERSION_REVISION 0

#ifdef _WIN64
#ifdef _DEBUG
    const char g_DllName[] = "tobii_gameintegration_x64_d.dll";
#else
    const char g_DllName[] = "tobii_gameintegration_x64.dll";
#endif
#else
#ifdef _DEBUG
    const char g_DllName[] = "tobii_gameintegration_x86_d.dll";
#else
    const char g_DllName[] = "tobii_gameintegration_x86.dll";
#endif
#endif

#if defined(GAME_INTEGRATION_EXPORT)
#   define TGI_TYPE_EXPORT __declspec(dllexport)
#   define TGI_C_EXPORT extern "C" __declspec(dllexport)
#else
#   define TGI_TYPE_EXPORT
#   define TGI_C_EXPORT
#endif

#include <cstdint>
#include <assert.h>

namespace TobiiGameIntegration
{
    struct ITobiiGameIntegrationApi;

#ifdef __cplusplus
    extern "C" 
    {
#endif
        // Please, specify a full, real game name, not a code name for the game.
        // Example: "Assassin's Creed Origins"

        TGI_C_EXPORT ITobiiGameIntegrationApi* __cdecl GetApi(const char* fullGameName, int majorVersion, int minorVersion, int revision);
        TGI_C_EXPORT ITobiiGameIntegrationApi* __cdecl GetApiDynamic(const char* fullGameName, const char* dllPath, int majorVersion, int minorVersion, int revision);
        
#ifdef __cplusplus
    }
#endif

    inline ITobiiGameIntegrationApi* GetApi(const char* fullGameName)
    {
        return GetApi(fullGameName, TGI_VERSION_MAJOR, TGI_VERSION_MINOR, TGI_VERSION_REVISION);
    }
    
    inline ITobiiGameIntegrationApi* GetApiDynamic(const char* fullGameName, const char* dllPath)
    {
        return GetApiDynamic(fullGameName, dllPath, TGI_VERSION_MAJOR, TGI_VERSION_MINOR, TGI_VERSION_REVISION);
    }
    
    inline ITobiiGameIntegrationApi* GetApiDynamic(const char* fullGameName)
    {
        return GetApiDynamic(fullGameName, nullptr, TGI_VERSION_MAJOR, TGI_VERSION_MINOR, TGI_VERSION_REVISION);
    }

    /* Common types */

    struct TGI_TYPE_EXPORT Vector2d
    {
        float X;
        float Y;
    };

    struct TGI_TYPE_EXPORT Vector3d
    {
        float X;
        float Y;
        float Z;
    };

    enum class HMDValidityFlags
    {
        LeftEyeIsValid = 1 << 0,
        RightEyeIsValid = 1 << 1
    };
    inline HMDValidityFlags operator |(HMDValidityFlags a, HMDValidityFlags b) { return static_cast<HMDValidityFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
    inline HMDValidityFlags& operator |=(HMDValidityFlags &a, HMDValidityFlags b) { return a = a | b; }
    inline HMDValidityFlags operator &(HMDValidityFlags a, HMDValidityFlags b) { return static_cast<HMDValidityFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }
    inline HMDValidityFlags& operator &=(HMDValidityFlags& a, HMDValidityFlags b) { return a = a & b; }

    struct TGI_TYPE_EXPORT EyeInfo
    {
        EyeInfo() :
            GazeOriginMM{ 0.0f, 0.0f, 0.0f },
            GazeDirection{ 0.0f, 0.0f, 0.0f },
            PupilPosition{ 0.0f, 0.0f },
            EyeOpenness(0.0f)
        { }

        Vector3d GazeOriginMM;
        Vector3d GazeDirection;
        Vector2d PupilPosition;
        float EyeOpenness;
    };

    struct TGI_TYPE_EXPORT HMDGaze
    {
        HMDGaze() :
            Timestamp(0xDEADBEEF),
            Validity(HMDValidityFlags::LeftEyeIsValid & HMDValidityFlags::RightEyeIsValid),
            Counter(0),
            LedMode(0),
            LeftEyeInfo(),
            RightEyeInfo()
        { }

        int64_t Timestamp;
        HMDValidityFlags Validity;
        uint32_t Counter;
        uint32_t LedMode;
        EyeInfo LeftEyeInfo;
        EyeInfo RightEyeInfo;
    };

    struct TGI_TYPE_EXPORT Quaternion
    {
        float X;
        float Y;
        float Z;
        float W;
    };

    struct TGI_TYPE_EXPORT Rectangle
    {
        int32_t Left;
        int32_t Top;
        int32_t Right;
        int32_t Bottom;
    };

    struct TGI_TYPE_EXPORT Dimensions
    {
        int32_t Width;
        int32_t Height;
    };

    enum UnitType
    {
        SignedNormalized = 0,   //  Gaze point, signed normalized, client window bottom, left = (-1, -1), client window top, right = (1, 1)
        Normalized,             //  Gaze point, unsigned normalized, client window bottom, left = (0, 0), client window top, right = (1, 1)
        Mm,                     //  Gaze point, mm, client window bottom, left = (0, 0), client window top, right = (window_width_mm, window_height_mm)
        Pixels,                 //  Gaze point, pixel, client window bottom, left = (0, 0), client window top, right = (window_width_pixels, window_height_pixels)
        NumberOfUnitTypes       //	Use for looping, array allocation etc 
    };


    /* Eye tracking specific types */

    struct TGI_TYPE_EXPORT GazePoint
    {
        int64_t TimeStampMicroSeconds;
        float X;
        float Y;

        GazePoint()
            : TimeStampMicroSeconds(0)
            , X(0.0f)
            , Y(0.0f)
        { }
    };

    struct TGI_TYPE_EXPORT Rotation
    {
        float Yaw;		// Clockwise rotation about the down vector, the angle increases when turning your head right
        float Pitch;	// Clockwise rotation about the right vector, the angle increases when turning your head up
        float Roll;		// Clockwise rotation about the forward vector, the angle increases when tilting your head to the right

        Rotation()
            : Yaw(0.0f)
            , Pitch(0.0f)
            , Roll(0.0f)
        { }
    };

    struct TGI_TYPE_EXPORT Position
    {
        float X;	// Increases when moving your head to the right
        float Y;	// Increases when moving your head up
        float Z;	// Increases when moving away from the tracker

        Position()
            : X(0.0f)
            , Y(0.0f)
            , Z(0.0f)
        { }
    };

    struct TGI_TYPE_EXPORT Transformation
    {
        Rotation Rotation;
        Position Position;

        Transformation()
            : Rotation()
            , Position()
        { }
    };

    struct TGI_TYPE_EXPORT HeadPose : Transformation
    {
        int64_t TimeStampMicroSeconds;

        HeadPose()
            : TimeStampMicroSeconds(0)
        { }
    };

    enum HeadViewType
    {
        None = 0,
        Direct = 1,
        Dynamic = 2
    };

    struct TGI_TYPE_EXPORT SensitivityGradientSettings
    {
        float Scale;            // Gradient curve for sensitivity settings
        float Exponent;
        float InflectionPoint;
        float StartPoint;
        float EndPoint;

        SensitivityGradientSettings()
            : Scale(1.0f)
            , Exponent(1.0f)
            , InflectionPoint(0.5f)
            , StartPoint(0.0f)
            , EndPoint(1.0f)
        {  }

        SensitivityGradientSettings(float scale, float exponent, float inflectionPoint, float startPoint, float endPoint)
            : Scale(scale),
            Exponent(exponent),
            InflectionPoint(inflectionPoint),
            StartPoint(startPoint),
            EndPoint(endPoint)
        { }
    };

    struct TGI_TYPE_EXPORT HeadViewAutoCenterSettings
    {
        bool IsEnabled;
        float NormalizeFasterGazeDeadZoneNormalized;
        float ExtendedViewAngleFasterDeadZoneDegrees;
        float MaxDistanceFromMasterCm;
        float MaxAngularDistanceDegrees;
        float FasterNormalizationFactor;
        float PositionCompensationSpeed;
        float RotationCompensationSpeed;

        HeadViewAutoCenterSettings()
            : IsEnabled(true),
            NormalizeFasterGazeDeadZoneNormalized(0.35f),
            ExtendedViewAngleFasterDeadZoneDegrees(10.0f),
            MaxDistanceFromMasterCm(5.0f),
            MaxAngularDistanceDegrees(15.0f),
            FasterNormalizationFactor(100.0f),
            PositionCompensationSpeed(0.01f),
            RotationCompensationSpeed(0.01f)
        { }
    };

    struct TGI_TYPE_EXPORT ExtendedViewSettings
    {
        HeadViewType HeadViewType;

        float AspectRatioCorrectionFactor;
        float HeadViewPitchCorrectionFactor;

        SensitivityGradientSettings GazeViewSensitivityGradientSettings;
        float GazeViewResponsiveness;
        float NormalizedGazeViewMinimumExtensionAngle;
        float NormalizedGazeViewExtensionAngle;

        SensitivityGradientSettings HeadViewSensitivityGradientSettings;
        float HeadViewResponsiveness;
        HeadViewAutoCenterSettings HeadViewAutoCenter;

        ExtendedViewSettings()
            : HeadViewType(HeadViewType::Direct)
            , AspectRatioCorrectionFactor(1.0f)
            , HeadViewPitchCorrectionFactor(1.5f)
            , GazeViewSensitivityGradientSettings(0.5f, 2.25f, 0.8f, 0.0f, 1.0f)
            , GazeViewResponsiveness(0.5f)
            , NormalizedGazeViewMinimumExtensionAngle(0.05f) // This is the minimal maximum eview angle from the gaze component. Normalized rotations (so 0.05 => 0.05 * 360 = 18 degrees)
            , NormalizedGazeViewExtensionAngle(NormalizedGazeViewMinimumExtensionAngle) // This is the maximum extension angle you can achieve with the gaze component.
            , HeadViewSensitivityGradientSettings(0.65f, 1.25f, 0.5f, 0.0f, 1.0f)
            , HeadViewResponsiveness(1.0f)
        
        { }

        ExtendedViewSettings(const ExtendedViewSettings &other)
        {
            *this = other;
        }
    };

    // This will generate enum Feature and corresponding FeatureNames array
#define FOREACH_FEATURE(FEATURE) \
            FEATURE(AutoPause)\
            FEATURE(CharacterAwareness)\
            FEATURE(EnvironmentalAwareness)\
            FEATURE(EyeContact)\
            FEATURE(ObjectAwareness)\
            FEATURE(BungeeZoom)\
            FEATURE(ZoomAtGaze)\
            FEATURE(CleanUI)\
            FEATURE(DirectionalSound)\
            FEATURE(DynamicDepthOfField)\
            FEATURE(DynamicLightAdaptation)\
            FEATURE(FlashlightControl)\
            FEATURE(PeripheralEffects)\
            FEATURE(AutoTurn)\
            FEATURE(ExtendedView)\
            FEATURE(FreeView)\
            FEATURE(AimAtGaze)\
            FEATURE(CoverAtGaze)\
            FEATURE(EnemyTagging)\
            FEATURE(InteractAtGaze)\
            FEATURE(ThrowAtGaze)\
            FEATURE(TeleportToGaze)\
            FEATURE(JumpAtGaze)\
            FEATURE(FireAtGaze)\
            FEATURE(SecondaryWeaponGaze)\
            FEATURE(PickUpAtGaze)\
            FEATURE(SelectAtGaze)\
            FEATURE(FoveatedRendering)\
            FEATURE(AvatarEyeMovements)\
            FEATURE(WheelMenu)\
            FEATURE(MenuNavigation)\
            FEATURE(CursorWarp)\
            FEATURE(CenterAtGaze)\

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

    enum class Feature {
        FOREACH_FEATURE(GENERATE_ENUM)
        AMOUNT_OF_FEATURES
    };

    static char const* FeatureNames[] = {
        FOREACH_FEATURE(GENERATE_STRING)
    };

    enum class TrackerType
    {
        None = 0,
        PC,
        HeadMountedDisplay
    };

    enum class CapabilityFlags
    {
        None = 0,
        Presence = 1 << 0,
        Head = 1 << 1,
        Gaze = 1 << 2,
        Foveation = 1 << 3,
        EyeInfo = 1 << 4,
        HMD = 1 << 5
    };
    inline CapabilityFlags operator |(CapabilityFlags a, CapabilityFlags b) { return static_cast<CapabilityFlags>(static_cast<int>(a) | static_cast<int>(b)); }
    inline CapabilityFlags& operator |=(CapabilityFlags &a, CapabilityFlags b) { return a = a | b; }
    inline CapabilityFlags operator &(CapabilityFlags a, CapabilityFlags b) { return static_cast<CapabilityFlags>(static_cast<int>(a) & static_cast<int>(b)); }
    inline CapabilityFlags& operator &=(CapabilityFlags& a, CapabilityFlags b) { return a = a & b; }

    struct TGI_TYPE_EXPORT AimAtGazeFilterSettings
    {
        AimAtGazeFilterSettings() :
            IsEnabled(false),
            DistanceThreshold(0.1f)
        {  }
        
        bool IsEnabled;
        float DistanceThreshold;
    };

    struct TGI_TYPE_EXPORT ResponsiveFilterSettings
    {
        ResponsiveFilterSettings() :
            IsEnabled(false),
            Responsiveness(0.5f),
            StickinessDistance(0.1f)
        {  }

        bool IsEnabled;
        float Responsiveness;
        float StickinessDistance;
    };

    struct TGI_TYPE_EXPORT TrackerInfo
    {
        TrackerInfo() :
            Type(TrackerType::None),
            Capabilities(CapabilityFlags::None),
            DisplayRectInOSCoordinates({ 0, 0, 0, 0}),
            DisplaySizeMm({ 0, 0 }),
            Url(nullptr),
            FriendlyName(nullptr),
            MonitorNameInOS(nullptr),
            ModelName(nullptr),
            Generation(nullptr),
            SerialNumber(nullptr),
            FirmwareVersion(nullptr),
            IsAttached(false)
        { }

        TrackerType Type;
        CapabilityFlags Capabilities;
        Rectangle DisplayRectInOSCoordinates;
        Dimensions DisplaySizeMm;
        const char* Url;
        const char* FriendlyName;
        const char* MonitorNameInOS;
        const char* ModelName;
        const char* Generation;
        const char* SerialNumber;
        const char* FirmwareVersion;
        bool IsAttached;
    };

    struct TGI_TYPE_EXPORT ITrackerController
    {
        virtual bool GetTrackerInfo(TrackerInfo& trackerInfo) = 0;
        virtual bool GetTrackerInfo(const char* url, TrackerInfo& trackerInfo) = 0;
        virtual void UpdateTrackerInfos() = 0;
        virtual bool GetTrackerInfos(const TrackerInfo*& trackerInfos, int& numberOfTrackerInfos) = 0;
        virtual bool TrackHMD() = 0;
        virtual bool TrackRectangle(const Rectangle& rectangle) = 0;
        virtual bool TrackWindow(void* windowHandle) = 0;
        virtual void StopTracking() = 0;
        virtual bool IsConnected() const = 0;
        virtual bool IsEnabled() const = 0;
    };

    struct TGI_TYPE_EXPORT IStreamsProvider
    {
        virtual bool GetLatestHeadPose(HeadPose& headPose) = 0;
        virtual bool GetLatestGazePoint(GazePoint& gazePoint) = 0;
        virtual int GetGazePoints(const GazePoint*& gazePoints) = 0;
        virtual int GetHeadPoses(const HeadPose*& headPoses) = 0;
        virtual bool GetLatestHMDGaze(HMDGaze& latestHMDGaze) = 0;
        virtual int GetHMDGaze(const HMDGaze*& hmdGaze) = 0;
        virtual bool IsPresent() = 0;
        virtual void ConvertGazePoint(const GazePoint& fromGazePoint, GazePoint& toGazePoint, UnitType fromUnit, UnitType toUnit) = 0;
        
        virtual void SetAutoUnsubscribeForCapability(CapabilityFlags capability, float timeout) = 0;
        virtual void UnsetAutoUnsubscribeForCapability(CapabilityFlags capability) = 0;
    };

    struct TGI_TYPE_EXPORT IExtendedView
    {
        virtual Transformation GetTransformation() const = 0;
        virtual void UpdateSettings(const ExtendedViewSettings& settings) = 0;
        virtual void UpdateGazeOnlySettings(const ExtendedViewSettings& settings) = 0;
        virtual void UpdateHeadOnlySettings(const ExtendedViewSettings& settings) = 0;
        virtual void ResetDefaultHeadPose() = 0;
    };

    struct TGI_TYPE_EXPORT IFeatures
    {
        virtual IExtendedView* GetExtendedView() = 0;
    };

    struct TGI_TYPE_EXPORT IFilters
    {
        virtual const ResponsiveFilterSettings& GetResponsiveFilterSettings() const = 0;
        virtual void SetResponsiveFilterSettings(ResponsiveFilterSettings settings) = 0;
        virtual const AimAtGazeFilterSettings& GetAimAtGazeFilterSettings() const = 0;
        virtual void SetAimAtGazeFilterSettings(AimAtGazeFilterSettings settings) = 0;
        
        virtual void GetResponsiveFilterGazePoint(GazePoint& gazePoint) const  = 0;
        virtual void GetAimAtGazeFilterGazePoint(GazePoint& gazePoint, float &gazePointStability) const = 0;
    };

    struct TGI_TYPE_EXPORT ITobiiGameIntegrationApi
    {
        virtual ITrackerController* GetTrackerController() = 0;
        virtual IStreamsProvider* GetStreamsProvider() = 0;
        virtual IFeatures* GetFeatures() = 0;
        virtual IFilters* GetFilters() = 0;

        virtual bool IsInitialized() = 0;
        virtual void Update() = 0;
        virtual void Shutdown() = 0;
    };
}