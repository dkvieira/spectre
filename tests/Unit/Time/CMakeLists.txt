# Distributed under the MIT License.
# See LICENSE.txt for details.

set(TIME_TESTS
    Time/Test_EveryNSlabs.cpp
    Time/Test_History.cpp
    Time/Test_PastTime.cpp
    Time/Test_Slab.cpp
    Time/Test_SpecifiedSlabs.cpp
    Time/Test_StepControllers.cpp
    Time/Test_Time.cpp
    Time/Test_TimeId.cpp
    )

add_subdirectory(Actions)
add_subdirectory(TimeSteppers)

set(SPECTRE_TESTS "${SPECTRE_TESTS};${TIME_TESTS}" PARENT_SCOPE)
set(LIBRARIES_TO_LINK "${LIBRARIES_TO_LINK};Time" PARENT_SCOPE)

