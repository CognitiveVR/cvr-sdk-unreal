# CLAUDE.md - Cognitive3D SDK for Unreal Engine

## Overview

This is the **Cognitive3D SDK for Unreal Engine**, an analytics and insights platform specifically designed for VR/XR applications. The SDK provides comprehensive telemetry, eye tracking, gaze analysis, user interaction tracking, and behavioral analytics for virtual reality experiences.

**Version:** 2.1.1
**License:** Cognitive3D SDK Software License (proprietary - see LICENSE file)
**Supported Unreal Versions:** 4.26, 4.27, 5.0, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6
**Documentation:** [docs.cognitive3d.com](http://docs.cognitive3d.com/unreal/get-started/)
**Support:** [Discord](https://discord.gg/x38sNUdDRH)

## Repository Structure

```
cvr-sdk-unreal/
├── Cognitive3DTest/                    # Test project demonstrating SDK usage
│   ├── Cognitive3DTest.uproject        # Unreal test project file
│   ├── Config/                         # Project configuration
│   ├── Content/                        # Test content and assets
│   ├── Plugins/
│   │   └── Cognitive3D/               # THE MAIN PLUGIN
│   │       ├── Cognitive3D.uplugin    # Plugin descriptor
│   │       ├── Content/               # Plugin content (widgets, textures, media)
│   │       ├── Resources/             # Plugin resources
│   │       └── Source/
│   │           ├── Cognitive3D/       # Runtime module (analytics engine)
│   │           │   ├── Public/        # Public headers (13 files)
│   │           │   ├── Private/       # Implementation
│   │           │   │   ├── C3DApi/    # Data recorders (events, gaze, sensors, etc.)
│   │           │   │   ├── C3DComponents/ # 22 component classes
│   │           │   │   ├── C3DNetwork/    # HTTP networking layer
│   │           │   │   ├── C3DRtaudio/    # Audio capture for surveys
│   │           │   │   └── C3DUtil/       # Utilities
│   │           │   └── Android/       # Android platform integration
│   │           └── Cognitive3DEditor/ # Editor module (tools, exporters, widgets)
│   │               ├── Public/        # Editor public headers
│   │               └── Private/       # Editor implementation
│   └── Source/                        # Test project source code
├── update*.py                         # Version-specific update scripts (4.26 - 5.6)
├── readme.md                          # Project overview
└── LICENSE                            # SDK license

```

## Key Concepts and Architecture

### Plugin Modules

The plugin consists of two Unreal modules:

1. **Cognitive3D (Runtime)**: Core analytics engine that runs in-game
2. **Cognitive3DEditor (Editor)**: Editor tools for scene export, configuration, and dynamic object management

### Core Architecture Components

#### 1. Analytics Provider
- **FAnalyticsCognitive3D**: Main module implementing `IAnalyticsProviderModule`
- **FAnalyticsProviderCognitive3D**: Central orchestrator implementing `IAnalyticsProvider`
- Manages session lifecycle, data recorders, and network communication

#### 2. Data Recorders (in `C3DApi/`)
- **FCustomEventRecorder**: Custom events with properties
- **FGazeDataRecorder**: HMD gaze and eye tracking data
- **FFixationDataRecorder**: Eye fixation detection and analysis
- **FSensors**: Time-series sensor data (FPS, battery, etc.)
- **FDynamicObjectManager**: Tracks spawned/moving objects
- **BoundaryRecorder**: VR guardian/boundary events

#### 3. Component System (22 components in `C3DComponents/`)

**Player Tracking:**
- `UPlayerTracker`: Main HMD position and gaze tracking
- `UFixationRecorder`: Eye tracking fixation detection

**Input Tracking:**
- `UInputTracker`: Legacy input system
- `UEnhancedInputTracker`: Enhanced Input system (UE5)

**VR Hardware:**
- `UHMDEvents`, `UHMDHeight`, `UHMDOrientation`, `UHMDRecenter`
- `UBoundaryEvent`, `UPassthrough`, `URoomSize`

**Sensors:**
- `UBatteryLevel`, `UFramerateSensor`, `UWifiSignal`

**Platform-Specific:**
- `USocialPlatform`, `UGliaSensorRecorder`, `UAndroidPlugin`

**Other:**
- `UMedia`: Media playback tracking
- `UDynamicObject`: Dynamic object component for runtime tracking

#### 4. Networking and Caching
- **FNetwork**: HTTP requests, uploads, retry logic
- **FLocalCache**: Offline data persistence, file-based queue

#### 5. Additional Features
- **Exit Polls**: Survey system with voice recording support
- **Remote Controls**: Fetch configuration variables for A/B testing
- **Active Session View**: Real-time debugging widget

### Data Flow

```
Game Events → Components → Data Recorders → Batching → Network → Cognitive3D Dashboard
                                    ↓
                              Local Cache (if offline)
```

### Supported Eye Tracking SDKs

The SDK supports multiple eye tracking systems via conditional compilation (see `Cognitive3D.Build.cs`):

- **Tobii Eye Tracking**
- **SRanipal** (Vive Pro Eye) - v1.2 and v1.3
- **Varjo** (up to v3.0.0)
- **Pico Mobile** / **PicoXR SDK**
- **HP Omnicept** (Glia)
- **Meta/Oculus** (Quest Pro)
- **OpenXR Eye Tracking**
- **WaveVR Eye Tracking**

## Development Workflow

### Enabling Platform Features

Eye tracking and VR platform features are **opt-in** via the Build.cs file:

**Location:** `Cognitive3DTest/Plugins/Cognitive3D/Source/Cognitive3D/Cognitive3D.Build.cs`

Uncomment the relevant function call to enable features:

```csharp
// Enable Meta/Oculus functionality
MetaXRPlugin();

// Enable Oculus Passthrough (UE 5.2+)
MetaXRPassthrough();

// Enable PICO XR SDK
PICOXR();

// Enable OpenXR Eye Tracking
OpenXREyeTracking();

// Enable Tobii Eye Tracking
TobiiEyeTracking();

// Enable Vive SRanipal
SRanipalVivePro();

// etc.
```

### Creating Release Builds for Different UE Versions

Python scripts are provided to create version-specific releases:

```bash
# For Unreal Engine 5.3
python update5_3.py

# For Unreal Engine 5.4
python update5_4.py
```

**What these scripts do:**
1. Copy plugin from `Cognitive3DTest/Plugins/` to temporary `Plugins/` directory
2. Delete Binaries/ and Intermediate/ folders
3. Modify Build.cs files for version compatibility:
   - Comment out unsupported platforms (e.g., Win32 in UE 5.3+)
   - Update Oculus plugin names (OculusHMD → OculusXRHMD for UE 5.3+)
   - Add necessary dependencies (XRBase, DeveloperToolSettings)
4. Create zip archive: `C3D_Plugin_<version>_ue5_3.zip`
5. Clean up temporary directory

### Common File Locations

**Configuration Files:**
- Plugin descriptor: `Cognitive3DTest/Plugins/Cognitive3D/Cognitive3D.uplugin`
- Runtime build: `Cognitive3DTest/Plugins/Cognitive3D/Source/Cognitive3D/Cognitive3D.Build.cs`
- Editor build: `Cognitive3DTest/Plugins/Cognitive3D/Source/Cognitive3DEditor/Cognitive3DEditor.Build.cs`

**Key Source Files:**
- Analytics provider: `Source/Cognitive3D/Public/Cognitive3DProvider.h`
- Blueprint API: `Source/Cognitive3D/Public/Cognitive3DBlueprints.h`
- Settings: `Source/Cognitive3D/Public/Cognitive3DSettings.h`
- Dynamic object: `Source/Cognitive3D/Public/DynamicObject.h`
- Custom events: `Source/Cognitive3D/Public/CustomEvent.h`

**Content:**
- Exit poll widgets: `Content/ExitpollPanels/`
- Active session view: `Content/Widgets/ActiveSessionView.uasset`

## API Reference

### Blueprint API

The primary interface for Blueprint users is `UCognitive3DBlueprints` (see `Cognitive3DBlueprints.h`).

**Session Management:**
```cpp
StartSession() -> bool
EndSession()
SetSessionName(FString name)
SetParticipantFullName(FString name)
SetParticipantId(FString id)
```

**Custom Events - Direct API:**
```cpp
SendCustomEvent(FString Name, TArray<FAnalyticsEventAttr> Attributes)
SendCustomEventPosition(FString Name, TArray<FAnalyticsEventAttr> Attributes, FVector Position)
```

**Custom Events - Fluent API:**
```cpp
UCustomEvent* event = MakeCustomEvent("player_died");
event = SetStringProperty(event, "cause", "enemy");
event = SetIntegerProperty(event, "level", 5);
event = SetPosition(event, playerLocation);
Send(event);
```

**Sensors:**
```cpp
InitializeSensor("temperature", 10.0f, 25.0f)  // name, hz, initial value
RecordSensor("temperature", 26.5f)
```

**Remote Controls (A/B Testing):**
```cpp
FetchRemoteControlVariableWithParticipantId(participantId)
float difficulty = GetRemoteControlVariableFloat("game_difficulty", 1.0f)
```

### C++ API

For C++ projects, access the provider directly:

```cpp
#include "Cognitive3D/Public/Cognitive3DProvider.h"

auto provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();
if (provider.IsValid())
{
    provider->StartSession(attributes);
    provider->SetSessionProperty("map_name", MapName);
    provider->customEventRecorder->Send("event_name", position, attributes);
}
```

### Component Usage

Add components to actors in editor or via code:

```cpp
// Add player tracker to camera/HMD
UPlayerTracker* playerTracker = NewObject<UPlayerTracker>(this);
playerTracker->RegisterComponent();

// Add dynamic object tracking
UDynamicObject* dynObj = NewObject<UDynamicObject>(spawned);
dynObj->MeshName = "enemy_type_1";
dynObj->RegisterComponent();
```

## Configuration

### Project Settings

Access via: **Project Settings > Analytics > Cognitive3D**

Key settings (`UCognitive3DSettings`):
- **Gateway**: API endpoint (default: `data.cognitive3d.com`)
- **EnableLogging**: General logging
- **EnableDevLogging**: Verbose logging for debugging
- **AutomaticallySetTrackingScene**: Auto-track on level load
- **LocalCacheSize**: Offline cache limit in MB (default: 100)
- **Batch Sizes**: Configure batch sizes for gaze (256), events (256), dynamic objects (512), sensors (512)
- **Auto Timers**: Configure auto-send intervals (default: 10 seconds)

### Engine.ini Configuration

Scene data and API keys are stored in `Config/DefaultEngine.ini`:

```ini
[Analytics]
ApplicationKey=your-application-key
DeveloperKey=your-developer-key
AttributionKey=your-attribution-key

[SceneName /Game/Maps/MainLevel]
SceneId=unique-scene-id-guid
VersionNumber=1
VersionId=version-id-number
```

## Testing and Building

### Building the Plugin

1. Open `Cognitive3DTest.uproject` in Unreal Editor
2. Let UE build the C++ plugin automatically
3. Verify in **Edit > Plugins > Analytics > Cognitive3D Analytics**

### Testing

The `Cognitive3DTest` project serves as both:
- **Development environment** for the plugin itself
- **Example implementation** showing SDK usage

**Test the plugin:**
1. Configure API keys in Project Settings
2. Open test maps in `Cognitive3DTest/Content/Maps/`
3. Enable "Enable Dev Logging" in settings
4. Play in VR or simulate
5. Check Output Log for Cognitive3D messages

### Active Session Debugging

Use the **Active Session View** widget to monitor live data:
- Current data counts (events, gaze points, dynamic objects)
- Network status
- Fixation visualization overlay
- Session information

## Common Tasks

### Adding a New Component Type

1. Create new class in `Source/Cognitive3D/Private/C3DComponents/`
2. Inherit from `UActorComponent`
3. Implement tick/event logic
4. Record data via `provider->customEventRecorder->Send()` or sensors
5. Add to `Cognitive3D.Build.cs` if needed
6. Document in header file

### Adding Eye Tracking for New Platform

1. Add conditional compilation block in `Cognitive3D.Build.cs`:
   ```cpp
   void NewPlatformEyeTracking() {
       PublicDefinitions.Add("INCLUDE_NEWPLATFORM_PLUGIN");
       PublicDependencyModuleNames.Add("NewPlatformSDK");
   }
   ```
2. Update `PlayerTracker.cpp` and `FixationRecorder.cpp` with `#if INCLUDE_NEWPLATFORM_PLUGIN` blocks
3. Implement platform-specific eye tracking API calls

### Exporting Scene Data

Use the Cognitive3D Editor tools:
1. Open **Cognitive3D > Scene Setup**
2. Configure scene export settings (decimation, texture quality)
3. Click "Export" to create GLTF and upload to dashboard
4. Scene ID is saved to DefaultEngine.ini automatically

### Creating Exit Polls

1. Configure question sets on Cognitive3D dashboard
2. In Blueprint, call `GetQuestionSet("hook_name", response_delegate)`
3. Display questions using provided widgets or custom UI
4. Collect answers and call `SendExitPollAnswers(answers)`

## Important Notes

### Platform Compatibility

- **Windows**: Full support (Win64 for UE 5.3+, Win32 deprecated)
- **Android**: Supported with platform-specific features (battery, WiFi)
- **VR Platforms**: Supports all major VR platforms via plugin dependencies

### Dependencies

**Required Plugins:**
- AnalyticsBlueprintLibrary (built-in)
- GLTFExporter (Marketplace - required for scene export)
- EnhancedInput (built-in)

**Optional Plugins (uncomment in Build.cs):**
- MetaXR (OculusXR)
- PicoXR
- SteamVR/OpenXR
- Tobii Eye Tracking
- Varjo
- WaveVR

### Performance Considerations

- Default batch sizes are optimized for typical VR experiences (60-90 FPS)
- Gaze tracking runs at 10 Hz (every 0.1 seconds) by default
- Data is batched and sent asynchronously to avoid frame drops
- Local cache prevents data loss during network issues
- Adjust batch sizes in Project Settings if experiencing performance issues

### Data Privacy

- All data is sent to Cognitive3D servers
- Participant IDs should be anonymized or hashed for GDPR compliance
- Voice recordings are captured as WAV and base64 encoded
- SDK license requires production agreement with Cognitive3D

## Troubleshooting

### "Module 'Cognitive3D' could not be loaded"

- Ensure Build.cs has correct dependencies for your UE version
- Check that platform-specific plugins (Oculus, Pico, etc.) are installed if enabled
- Rebuild the project from Visual Studio/Rider

### Eye Tracking Not Working

- Verify the correct eye tracking SDK is installed
- Uncomment the appropriate function in `Cognitive3D.Build.cs`
- Check HMD is properly initialized before session starts
- Enable dev logging to see eye tracking initialization messages

### Data Not Uploading

- Verify API keys in Project Settings > Analytics
- Check Gateway URL is correct (default: `data.cognitive3d.com`)
- Look for network errors in Output Log
- Data is cached locally if offline - will retry on reconnection

### Scene Not Appearing in Dashboard

- Run Scene Setup in editor to export and upload
- Verify Scene ID in DefaultEngine.ini matches dashboard
- Check developer key is valid

## Resources

- **Documentation**: [docs.cognitive3d.com](http://docs.cognitive3d.com/unreal/get-started/)
- **Discord Community**: [discord.gg/x38sNUdDRH](https://discord.gg/x38sNUdDRH)
- **GitHub Issues**: Submit issues and feature requests here
- **Dashboard**: [cognitive3d.com](https://cognitive3d.com) - Configure projects and view analytics

## Development Tips for Claude

1. **Always check Build.cs first** when dealing with compilation errors - platform features are conditionally compiled
2. **The test project IS the plugin** - development happens in `Cognitive3DTest/Plugins/Cognitive3D/`
3. **Two modules**: Runtime (Cognitive3D) and Editor (Cognitive3DEditor) - keep them separate
4. **Update scripts**: When making changes for a specific UE version, update the corresponding `update*.py` script
5. **Blueprint exposure**: Most runtime APIs should be exposed via `Cognitive3DBlueprints.h` for Blueprint users
6. **Data batching**: All data recorders use batching - understand the batch size limits in Settings
7. **Component-based**: Most features are implemented as UActorComponents for easy attachment
8. **Eye tracking**: Multiple SDKs supported via conditional compilation - check preprocessor defines
9. **Async everything**: Network calls are async, use callbacks/delegates appropriately
10. **Cache-first**: Data goes to local cache first, then uploads - handle both code paths

---

**Last Updated**: 2025-11-18 (based on SDK v2.1.1)
