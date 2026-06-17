plugins {
    id("com.android.application")
}

android {
    namespace = "net.dot.clrdemo"
    compileSdk = 35

    defaultConfig {
        applicationId = "net.dot.clrdemo"
        minSdk = 21
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"

        ndk {
            abiFilters += "arm64-v8a"
        }

        externalNativeBuild {
            cmake {
                val src = if (project.hasProperty("dotnetRuntimeSrc"))
                    project.property("dotnetRuntimeSrc").toString()
                else "E:/Code/DotNet"

                val pub = if (project.hasProperty("dotnetPublishDir"))
                    project.property("dotnetPublishDir").toString()
                else "${rootProject.projectDir}/publish"

                arguments += "-DDOTNET_RUNTIME_SRC=$src"
                arguments += "-DDOTNET_PUBLISH_DIR=$pub"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
}
