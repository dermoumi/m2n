Create a local.properties file with the following content:

	sdk.dir=/path/to/your/android/sdk
	ndk.dir=/path/to/your/android/ndk

Compile with

	./gradlew assembleDebug
	./gradlew installDebug

or

	./gradlew assembleRelease
