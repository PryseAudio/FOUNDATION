# ==================================================================================================
# FOUNDATION — plugin identity
#
#   ALL VALUES BELOW ARE PLACEHOLDERS (TBD). DO NOT SHIP A BUILD WITH THEM.
#
# The plugin code and manufacturer code become permanent the moment a build leaves the development
# machine: hosts store them in saved sessions, so changing them later breaks session recall.
# Replace every value here with the final identity before any external/public build.
#
# Format rules (enforced by JUCE / Apple):
#   - PLUGIN_MANUFACTURER_CODE: 4 chars, at least one upper-case; GarageBand requires the first
#     letter upper-case and the remaining letters lower-case.
#   - PLUGIN_CODE: 4 chars, exactly one upper-case letter (AU compatibility).
# The leading "X" marks both codes as placeholders.
# ==================================================================================================

set(FOUNDATION_COMPANY_NAME             "PLACEHOLDER_COMPANY")        # TBD
set(FOUNDATION_BUNDLE_ID                "com.placeholder.foundation") # TBD
set(FOUNDATION_PLUGIN_MANUFACTURER_CODE "Xplh")                       # TBD
set(FOUNDATION_PLUGIN_CODE              "Xfdn")                       # TBD
set(FOUNDATION_PRODUCT_NAME             "FOUNDATION")
set(FOUNDATION_PLUGIN_NAME              "FOUNDATION")

# The product version comes from project(FOUNDATION VERSION ...) in the root CMakeLists.txt.
# It is deliberately independent of the state schemaVersion (12_PARAMETERS_STATE/StateSerializer).
