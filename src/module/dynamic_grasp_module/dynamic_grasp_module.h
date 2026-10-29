#pragma once

#include "aimrt_module_cpp_interface/module_base.h"

namespace genisys_grasp_decision::dynamic_grasp_module {

class DynamicGraspModule : public aimrt::ModuleBase {
 public:
  DynamicGraspModule() = default;
  ~DynamicGraspModule() override = default;

  aimrt::ModuleInfo Info() const override {
    return aimrt::ModuleInfo{.name = "DynamicGraspModule"};
  }

  bool Initialize(aimrt::CoreRef core) override;

  bool Start() override;

  void Shutdown() override;

 private:
  auto GetLogger() { return core_.GetLogger(); }

 private:
  aimrt::CoreRef core_;
};

}  // namespace genisys_grasp_decision::dynamic_grasp_module
