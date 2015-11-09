Create a local.properties file with the following content:

	sdk.dir=/path/to/your/android/sdk
	ndk.dir=/path/to/your/android/ndk

Compile and install debug with

	./gradlew assembleDebug installDebug

Requires Android SDK Build Tools revision 22.0.1