#include <cstring>

#include "aimrt_pkg_c_interface/pkg_macro.h"
#include "dynamic_grasp_module/dynamic_grasp_module.h"
#include "static_grasp_module/static_grasp_module.h"

static std::tuple<std::string_view, std::function<aimrt::ModuleBase*()>>
    aimrt_module_register_array[]{
        {"StaticGraspModule",
         []() -> aimrt::ModuleBase* {
           return new genisys_grasp_decision::static_grasp_module::StaticGraspModule();
         }},
        {"DynamicGraspModule",
         []() -> aimrt::ModuleBase* {
           return new genisys_grasp_decision::dynamic_grasp_module::DynamicGraspModule();
         }},
    };

AIMRT_PKG_MAIN(aimrt_module_register_array)
