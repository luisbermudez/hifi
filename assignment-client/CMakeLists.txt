set(TARGET_NAME assignment-client)

setup_hifi_project(Core Gui Network Script Quick Widgets WebSockets)

# link in the shared libraries
link_hifi_libraries( 
  audio avatars octree environment gpu model fbx entities 
  networking animation recording shared script-engine embedded-webserver
  controllers physics
)

include_application_version()
package_libraries_for_deployment()
