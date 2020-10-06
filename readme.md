# NanoProtoSheld
==============
## This is just the beginning of a library for interacting with the NanoProtoSheld Rev. 3.0. Its purpose is to be an educational tool for young students of computer science and electronics.

# API (Application Programing Interface)
==============
## class NanoProtoShield
### NanoProtoShield(FEATURES features = FEATURE_ALL);
Class constructor. Initializes all the member classes identified by the features list passed in
except for the temperature one because we can save the RAM from having that one always allocated by
just putting it on the stack when we want to take a reading.

### void begin(INDEX_PINS pinout[] = NULL);
begin() should be called in the setup() part of any script that uses a global NanoProtoShield object.
Sets up all the I/O pins and begins the member classes and prepares them for use.
