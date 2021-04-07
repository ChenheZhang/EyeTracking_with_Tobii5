/* Copyrighted Tobii AB, 2018. Property of Tobii AB. */

/**
## tobii_g2om.h

The tobii_g2om.h header file collects the core API functions of the Tobii G2OM library. It contains
functions to initialize the API and process data to get a most likely candidate result.

The API documentation includes example code snippets that show the use of each function, they don't
necessarily describe the best practice in which to use the api. For more in-depth example of the
best practices, see the examples that are supplied together with the G2OM library.

G2OM has no processes running in background, zero runtime allocations and enables full control
over the amount of threads used internally.
*/

#ifndef tobii_g2om_h_included
#define tobii_g2om_h_included

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #define G2OM_API __declspec( dllimport )
    #define G2OM_CALL __cdecl
#elif __GNUC__ >= 4
    #define G2OM_API __attribute__((visibility("default")))
    #define G2OM_CALL
#else
    #define G2OM_API
    #define G2OM_CALL
#endif

typedef enum {
    G2OM_RESULT_OK = 0,
    G2OM_RESULT_NULL_POINTER_PASSED = -1,
    G2OM_RESULT_INTERNAL_ERROR = -2,
    G2OM_RESULT_THREAD_POOL_ERROR = -3,
    G2OM_RESULT_INDEX_OUT_OF_BOUNDS = -4,
    G2OM_RESULT_INTERNAL_CONVERSION_ERROR = -5,
    G2OM_RESULT_LOG_FILE_ERROR = -6,
    G2OM_RESULT_NOT_PERMITTED_BY_LICENSE = -7,
    G2OM_RESULT_LICENSE_INVALID = -8,
    G2OM_RESULT_NOT_IMPLEMENTED = -9,
    G2OM_RESULT_CAPACITY_EXCEEDED = -10,
    G2OM_RESULT_CUSTOM_ALLOCATOR_NOT_SET_FIRST = -11,
} g2om_result;

typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t build_version;
} g2om_version;

typedef struct g2om_context g2om_context;

typedef struct {
    float x;
    float y;
    float z;
} g2om_vector;

typedef struct {
    uint32_t capacity;
    uint32_t thread_count;
    uint8_t *license_ptr;
    uint32_t license_length;
} g2om_context_create_options;

typedef struct {
    float data[16];
} g2om_matrix;

typedef struct {
    uint64_t id;
    g2om_vector max_local_space;
    g2om_vector min_local_space;
    g2om_matrix world_to_local_matrix;
    g2om_matrix local_to_world_matrix;
} g2om_candidate;

typedef struct {
    g2om_vector origin;
    g2om_vector direction;
} g2om_ray;

typedef struct {
    g2om_ray ray;
    uint8_t is_valid;
} g2om_gaze_ray;

typedef struct {
    float timestamp_in_s;
    g2om_gaze_ray gaze_ray_world_space;
    g2om_vector camera_up_direction_world_space;
    g2om_vector camera_right_direction_world_space;
} g2om_gaze_data;

typedef struct {
    uint8_t is_valid;
    uint64_t id;
} g2om_raycast;

typedef struct {
    g2om_raycast raycast;
} g2om_raycast_result;

typedef struct {
    uint64_t id;
    float score;
    g2om_gaze_ray adjusted_gaze_ray_world_space;
} g2om_candidate_result;

typedef enum {
    G2OM_CORNERS_FLL = 0,    // Front Lower Left
    G2OM_CORNERS_FUL,        // Front Upper Left
    G2OM_CORNERS_FUR,        // Front Upper Right
    G2OM_CORNERS_FLR,        // Front Lower Right
    G2OM_CORNERS_BLL,        // Back Lower Left
    G2OM_CORNERS_BUL,        // Back Upper Left
    G2OM_CORNERS_BUR,        // Back Upper Right
    G2OM_CORNERS_BLR,        // Back Lower Right
    G2OM_CORNERS_COUNT
} g2om_corners;

G2OM_API g2om_result G2OM_CALL g2om_get_version(g2om_version *version_ptr);

G2OM_API g2om_result G2OM_CALL g2om_context_options_init(g2om_context_create_options *options);

G2OM_API g2om_result G2OM_CALL g2om_context_create(g2om_context **context_ptr);

G2OM_API g2om_result G2OM_CALL g2om_context_create_ex(g2om_context **context_ptr, g2om_context_create_options *options);

G2OM_API g2om_result G2OM_CALL g2om_context_destroy(g2om_context **context_ptr);

G2OM_API g2om_result G2OM_CALL g2om_process(
    g2om_context *context_ptr,
    g2om_gaze_data *gaze_data_ptr,
    g2om_raycast_result *raycast_ptr,
    uint32_t candidates_count,
    g2om_candidate *candidates_ptr,
    g2om_candidate_result *candidate_results);

G2OM_API g2om_result G2OM_CALL g2om_get_worldspace_corner_of_candidate(
    g2om_candidate *candidate_ptr,
    uint32_t corners_length,
    g2om_vector *corners_ptr);

G2OM_API g2om_result G2OM_CALL g2om_get_candidate_search_pattern(
    g2om_context *context_ptr,
    g2om_gaze_data *gaze_data_ptr,
    uint32_t number_of_rays,
    g2om_gaze_ray *mutated_rays_ptr);

#ifdef __cplusplus
}
#endif

#endif // tobii_g2om_h_included

/**

***

## Types

*/

/**

### g2om_result

#### Definition

```c,ignore
typedef enum {
    G2OM_RESULT_OK = 0,
    G2OM_RESULT_NULL_POINTER_PASSED = -1,
    G2OM_RESULT_INTERNAL_ERROR = -2,
    G2OM_RESULT_THREAD_POOL_ERROR = -3,
    G2OM_RESULT_INDEX_OUT_OF_BOUNDS = -4,
    G2OM_RESULT_INTERNAL_CONVERSION_ERROR = -5,
    G2OM_RESULT_LOG_FILE_ERROR = -6,
    G2OM_RESULT_NOT_PERMITTED_BY_LICENSE = -7,
    G2OM_RESULT_LICENSE_INVALID = -8,
    G2OM_RESULT_NOT_IMPLEMENTED = -9,
    G2OM_RESULT_CAPACITY_EXCEEDED = -10,
    G2OM_RESULT_CUSTOM_ALLOCATOR_NOT_SET_FIRST = -11
} g2om_result;
 ```

#### Remarks

The `g2om_result` type is returned by every `g2om_` call. It is used to signal success, or the
 cause of an error. The following variants can be observed:

- `G2OM_RESULT_OK` - The call was generally successful.
- `G2OM_RESULT_NULL_POINTER_PASSED` - A null pointer was passed at any position.
- `G2OM_RESULT_INTERNAL_ERROR` - This should never happen. If you observe this result please contact support.
- `G2OM_RESULT_THREAD_POOL_ERROR` - There was a problem creating the requested number of threads.
- `G2OM_RESULT_INDEX_OUT_OF_BOUNDS` - The library tried to access an element past its length. Make sure the passed pointers and lengths match.
- `G2OM_RESULT_INTERNAL_CONVERSION_ERROR` - This should never happen. If you observe this result please contact support.
- `G2OM_RESULT_LOG_FILE_ERROR` - When debug mode was enabled and the log file could not be written.
- `G2OM_RESULT_NOT_PERMITTED_BY_LICENSE` - When the current license does not allow the requested operation.
- `G2OM_RESULT_LICENSE_INVALID` - If an invalid or corrupted license is passed.
- `G2OM_RESULT_NOT_IMPLEMENTED` - This should never happen for public versions. If you observe this result please contact support.
- `G2OM_RESULT_CAPACITY_EXCEEDED` - If attempting to classify more objects at once than the provisioned capacity.
- `G2OM_RESULT_CUSTOM_ALLOCATOR_NOT_SET_FIRST` - If trying to set the custom allocator too late.

*/

/**

### g2om_version

#### Definition

```c,ignore
typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t build_version;
} g2om_version;
```

#### Remarks

When invoking `g2om_get_version()`, this struct will be filled.

*/

/**

### g2om_context

#### Definition

```c,ignore
typedef struct g2om_context g2om_context;
```

#### Remarks

An opaque struct for internal use. This will be created by `g2om_context_create()`
 or `g2om_context_create_ex()` and must be passed via pointer to most function calls.

*/

/**

### g2om_context_create_options

#### Definition

```c,ignore
typedef struct {
    uint32_t capacity;
    uint32_t thread_count;
    uint8_t *license_ptr;
    uint32_t license_length;
} g2om_context_create_options;
```

#### Remarks

When invoking `g2om_context_create_ex()` this specifies how G2OM should behave. The fields used
 are:

- `capacity` - How many concurrently existing objects G2OM should be able to handle.
- `thread_count` - When processing, how many parallel threads should be used. Setting this to `1`
 will not spawn any threads and will always execute on the calling thread of `g2om_process()`.
- `license_ptr` - A pointer to the raw bytes of a license (if any).
- `license_length` - Length of the license array.

*/

/**

### g2om_vector

#### Definition

```c,ignore
typedef struct {
    float x;
    float y;
    float z;
} g2om_vector;
```

#### Remarks

Our vector definition.

*/

/**

### g2om_matrix

#### Definition

```c,ignore
typedef struct {
    float data[16];
} g2om_matrix;
```

#### Remarks

Our matrix definition is identical to what Unity have, see Unity Matrix4x4.  
Memory layout (column major):

|   | c0 | c1 | c2 | c3 |
|---|---|---|---|---|
| r0 | m00 | m10 | m20 | m30 |
| r1 | m01 | m11 | m21 | m31 |
| r2 | m02 | m12 | m22 | m32 |
| r3 | m03 | m13 | m23 | m33 |

Data is stored in memory:
m00, m01, m02, m03 .. m33

*/

/**

### g2om_candidate

#### Definition

```c,ignore
typedef struct {
    uint64_t id;
    g2om_vector max_local_space;
    g2om_vector min_local_space;
    g2om_matrix world_to_local_matrix;
    g2om_matrix local_to_world_matrix;
} g2om_candidate;
```

#### Remarks

One object in the current scene that might or might not be gazed upon. This is input to
 `g2om_process()`. The fields used are:

- `id` - A user-defined ID of the object to track.
- `max_local_space` - A point describing the max point of the object's extent in local space
- `min_local_space` - A point describing the min point of the object's extent in local space
- `world_to_local_matrix` - A matrix that transforms a point from world space into object's local space
- `local_to_world_matrix` - A matrix that transforms a point from object's local space into world space

*/

/**

### g2om_ray

#### Definition

```c,ignore
typedef struct {
    g2om_vector origin;
    g2om_vector direction;
} g2om_ray;
```

#### Remarks

This is a ray that has an `origin` from the ray was shot, and a `direction` towards which it
 was shot.

- `origin` - The origin of the ray.
- `direction` - The direction of the ray.

*/

/**

### g2om_gaze_ray

#### Definition

```c,ignore
typedef struct {
    g2om_ray ray;
    uint8_t is_valid;
} g2om_gaze_ray;
```

#### Remarks

This is gaze ray as returned by an eye tracker. In addition to the ray data, it also contains
 a validity flag. The validity indicates whether the eye tracker thinks this ray was valid.

 The ray must be set in world space coordinates.

- `ray` - The ray as returned by the eye tracker, converted to world space.
- `is_valid` - The validity flag as reported by the eye tracker.

*/

/**

### g2om_gaze_data

#### Definition

```c,ignore
typedef struct {
    float timestamp_in_s;
    g2om_gaze_ray gaze_ray_world_space;
    g2om_vector camera_up_direction_world_space;
    g2om_vector camera_right_direction_world_space;
} g2om_gaze_data;
```

#### Remarks

Represents all the gaze data as returned from the eye tracker. The gaze data must be converted to
 world space coordinates.

- `timestamp_in_s` - Time stamp as reported by the eye tracker converted to seconds. Note that
 the absolute time is irrelevant, but the relative time passed in two consecutive invocations is
 important.
- `gaze_ray_world_space` - The eye data as reported by the eye tracker.
- `camera_up_direction_world_space` - The cameras upwards direction in world space coordinates.
- `camera_right_direction_world_space` - The cameras rightwards direction in world space coordinates.

*/

/**

### g2om_raycast

#### Definition

```c,ignore
typedef struct {
    uint8_t is_valid;
    uint64_t id;
} g2om_raycast;
```

#### Remarks

The result of a single ray cast.

- `is_valid` - If the ray cast data is actually valid. A `false` value means for example that
 the eye tracker did not produce a gaze ray this frame, or that the gaze ray did not hit any object.
- `id` - The ID of the object that was hit by the ray cast.

*/

/**

### g2om_raycast_result

#### Definition

```c,ignore
typedef struct {
    g2om_raycast raycast;
} g2om_raycast_result;
```

#### Remarks

The result of ray cast by the current gaze data.

- `raycast` - Result of raycasting the world gaze ray in game engine.

*/

/**

### g2om_candidate_result

#### Definition

```c,ignore
typedef struct {
    uint64_t id;
    float score;
    g2om_gaze_ray adjusted_gaze_ray_world_space;
} g2om_candidate_result;
```

#### Remarks

For each object returned by `g2om_process()` the following fields will be set.

- `id` - The user defined ID of the object this struct describes.
- `score` - The score of this object. As of today this is _almost_ a probability value, in that it
 ranges from `0` to `1`, with a higher value indicating a higher likelihood of being selected. A value
 of 0 means the object has not been selected.
- `adjusted_gaze_ray_world_space` - The fused left- and right-eye data adjusted to hit the closest point on the object.

*/

/**

***

## Functions

*/

/**

### g2om_get_version()

Get the current version of the API.

#### Definition

```c,ignore
g2om_result g2om_get_version(g2om_version *version_ptr);
```

#### Remarks

`g2om_get_version()` can be used to query the version of the `tobii_g2om` library currently used.

#### Arguments

- `version_ptr` is a pointer to a `g2om_version` variable to receive the current version numbers.

### Return value

A `g2om_result` is returned.

#### Example

```c
#include <tobii/tobii_g2om.h>
#include <stdio.h>
int main()
{
    g2om_version version;
    g2om_result result = g2om_get_version( &version );

    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    return 0;
}
```
*/

/**

### g2om_context_options_init()

When using `g2om_context_options_init`, this function initializes the  `g2om_context_create_options`
 structure to default values.

#### Definition

```c,ignore
g2om_result g2om_context_options_init(g2om_context_create_options *options);
```

#### Arguments

- `options` is a pointer to a `g2om_context_create_options` structure to be initialized.

#### Return value

A `g2om_result` is returned.

#### Example

```c
#include <tobii/tobii_g2om.h>
#include <stdio.h>
int main()
{
    g2om_context_create_options options;
    g2om_result result = g2om_context_options_init( &options );

    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    return 0;
}
```
*/

/**

### g2om_context_create()

Initializes a G2OM instance with default parameters. This is a wrapper function around
 `g2om_context_create_ex()`.

#### Definition

```c,ignore
g2om_result g2om_context_create(g2om_context **context_ptr);
```

#### Remarks

For details, please see `g2om_context_create_ex()`.

#### Arguments

- `context_ptr` must be a pointer to a variable of the type `g2om_context*` that is, a pointer to a `g2om_context` pointer. This variable will be filled in with a pointer to the created instance. It is an opaque type, and only its declaration is available in the API.

#### Return value

A `g2om_result` is returned.

#### Example

```c
#include <tobii/tobii_g2om.h>
#include <stdio.h>
int main()
{
    g2om_context* context;
    g2om_result result = g2om_context_create( &context );
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    result = g2om_context_destroy( &context );
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    return 0;
}
```
*/

/**

### g2om_context_create_ex()

Initializes a G2OM instance in extended mode. It is possible to have multiple G2OM instances at one time.

#### Definition

```c,ignore
g2om_result g2om_context_create_ex(g2om_context **context_ptr, g2om_context_create_options *options);
```

#### Remarks

Before any other API function can be invoked that accepts a `context` parameter the API needs to be
set up before use. The functions `g2om_context_create()` and `g2om_context_create_ex()` serve this purpose.
They allocate memory and set up internal data structures.

The resulting `g2om_context` instance can then be passed explicitly to most functions.

#### Arguments

- `context_ptr` - Must be a pointer to a variable of the type `g2om_context*` that is, a pointer to a`g2om_context` pointer. This variable will be filled in with a pointer to the created instance. It is an opaque type, and only its declaration is available in the API.
- `options` - A pointer to the `g2om_context_create_options` structure. The structure must be initialized with `g2om_context_options_init()`.

#### Return value

A `g2om_result` is returned.

#### Example

```c
#include <tobii/tobii_g2om.h>
#include <stdio.h>
int main()
{
    g2om_context* context;
    g2om_context_create_options options;

    g2om_result result = g2om_context_options_init(&options);
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    options.capacity = 5;
    options.thread_count = 1;

    result = g2om_context_create_ex( &context, &options);
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    result = g2om_context_destroy( &context );
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    return 0;
}
```
*/

/**

### g2om_context_destroy()

Destroy the G2OM instance.

#### Definition

```c,ignore
g2om_result g2om_context_destroy(g2om_context **context_ptr);
```

#### Remarks

When creating an instance with `g2om_context_create()` or `g2om_context_create_ex()`, some system resources are acquired. When finished
 using the API (typically during the shutdown process), `g2om_context_destroy()` should be called to destroy
 the instance and ensure that those resources are released. The function `g2om_context_destroy()`
 must only be called if the `g2om_context_create_*` function completed successfully.

#### Arguments

- `context_ptr` must be a pointer to a pointer to a valid `g2om_context` instance, that is a pointer
 to a `g2om_context*`, as created by `g2om_context_create_*()`.

#### Return value

A `g2om_result` is returned.

#### Example

See `g2om_context_create()`.

*/

/**

### g2om_process()

Takes gaze data, information about objects hit by raycasts and candidates (gaze aware objects) and
 sends back a result for each candidate.

#### Definition

```c,ignore
g2om_result g2om_process(
    g2om_context *context_ptr,
    g2om_gaze_data *gaze_data_ptr,
    g2om_raycast_result *raycast_ptr,
    uint32_t candidates_count,
    g2om_candidate *candidates_ptr,
    g2om_candidate_result *candidate_results);
```

#### Remarks

G2OM does not do any kind of background processing. This means that in order to receive any results
 from the API, the user application needs to manually initiate the process, and this is done by
 calling `g2om_process()`. The recommended way is to call it directly from
 the users update loop before the result is needed and at the same rate as the user application will
 use it.

This function is guaranteed not to allocate.

#### Arguments

- `context_ptr` must be a pointer to a valid `g2om_context` instance as created by calling `g2om_context_create_*()`.
- `gaze_data_ptr` must be a pointer to a valid `g2om_gaze_data` instance as created by the user.
- `raycast_ptr` must be a pointer to a valid `g2om_raycast_result` instance as created by the user.
- `candidates_count` must be a u32 designating the length of the `candidates_ptr` array.
- `candidates_ptr` must be a pointer to a valid `g2om_candidate` array instance as created by the user.
- `candidate_results` must be a pointer to a valid `g2om_candidate_result` array instance as created by the
 user, which will receive it's values from this function. `candidate_results` length **MUST** match
 `candidates_ptr` length.

#### Return value

A `g2om_result` is returned.

#### Example

```c
#include <tobii/tobii_g2om.h>
#include <stdio.h>
int main()
{
    g2om_context* context;
    g2om_result result = g2om_context_create( &context );

    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    // Get this data from your eye tracker:
    g2om_gaze_data gaze_data = { 0 };

    // Fill with data from ray casting in your game engine with gaze ray
    // should only be valid if object hit was gazeaware / g2om_candidate
    g2om_raycast_result raycast_result;

    // fill with candidates/objects from your game which should be gazeaware.
    g2om_candidate candidate = { 0 };
    g2om_candidate candidates[1] = {
        candidate
    };

    g2om_candidate *candidates_ptr = candidates;

    // this value will be filled by g2om_process
    g2om_candidate_result candidate_result = { 0 };

    // this array has to be the same size as candidates.
    g2om_candidate_result candidate_results[1] = {
        candidate_result
    };

    g2om_candidate_result *candidate_results_ptr = candidate_results;

    uint32_t candidate_count = 1;
    uint32_t result_count = 1;

    if (result_count != candidate_count) {
        printf("Mismatch in array lengths %i %i !", result_count, candidate_count);
    }

    result = g2om_process( context, &gaze_data, &raycast_result, candidate_count, candidates_ptr, candidate_results_ptr );
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    // --> use the results/winner here, e.g.: <--
    // g2om_candidate_result winner = candidate_results[0]; // the results are sorted, so the first element is the highest scoring.
    // printf("%ju has gaze focus.", winner.id);

    result = g2om_context_destroy( &context );
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    return 0;
}
```
*/

/**

### g2om_get_worldspace_corner_of_candidate()

Transforms the corner points of the provided candidate's AABB to worldspace, and output the
 resulting 8 corners.


#### Definition

```c,ignore
g2om_result g2om_get_worldspace_corner_of_candidate(
    g2om_candidate *candidate_ptr,
    uint32_t corners_length,
    g2om_vector *corners_ptr);
```

#### Remarks

`g2om_get_worldspace_corner_of_candidate()` can be used to debug. It's use is to make sure that
 the matrices used to transform to and from worldspace are correct.

#### Arguments

- `candidate_ptr` must be a pointer to a valid `g2om_candidate` instance as created by the user.
- `corners_ptr` is a pointer to a `g2om_vector` array, which must be of size 8, to receive the transformed corners.

#### Return value

A `g2om_result` is returned.

#### Example

```c
#include <tobii/tobii_g2om.h>
#include <stdio.h>
int main()
{
    // Made up information
    g2om_candidate candidate = {0};
    uint32_t number_of_corners = G2OM_CORNERS_COUNT;
    g2om_vector corners[G2OM_CORNERS_COUNT];
    g2om_result result = g2om_get_worldspace_corner_of_candidate( &candidate, number_of_corners, corners );
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    return 0;
}
```
*/

/**

### g2om_get_candidate_search_pattern()

Gets an optimized search pattern rays to be used to find candidates.

#### Definition

```c,ignore
G2OM_API g2om_result G2OM_CALL g2om_get_candidate_search_pattern(
    g2om_context *context_ptr,
    g2om_gaze_data *gaze_data_ptr,
    uint32_t number_of_rays,
    g2om_gaze_ray *mutated_rays_ptr,
);
```

#### Remarks

This only gets the rays. Finding the candidates has to be done by the application itself.

#### Arguments

TODO!

#### Return value

A `g2om_result` is returned.

#### Example

```c
#include <tobii/tobii_g2om.h>
#include <stdio.h>

#define number_of_rays 9

int main()
{
    g2om_context* context;
    g2om_result result = g2om_context_create( &context );
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
    }

    g2om_gaze_data gaze_data = {0};
    g2om_gaze_ray search_pattern[number_of_rays];

    result = g2om_get_candidate_search_pattern(context, &gaze_data, number_of_rays, search_pattern);
    if( result != G2OM_RESULT_OK ) {
        printf("Operation failed with error code: %d\n", result);
        return result;
     }

    return 0;
}
```
*/