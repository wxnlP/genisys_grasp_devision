#pragma once

#include "aimrt_module_cpp_interface/module_base.h"
#include "aimrt_module_protobuf_interface/channel/protobuf_channel.h"
#include "IKCheck.aimrt_rpc.srv.h"
#include "graspnet_candidate.aimrt_rpc.pb.h"
#include "pose.pb.h"
#include <optional>
#include <mutex>

namespace genisys_grasp_decision::static_grasp_module {

using PoseStamped = genisys::protocols::PoseStamped;

class StaticGraspModule : public aimrt::ModuleBase {
 public:
  StaticGraspModule() = default;
  ~StaticGraspModule() override = default;

  aimrt::ModuleInfo Info() const override {
    return aimrt::ModuleInfo{.name = "StaticGraspModule"};
  }

  bool Initialize(aimrt::CoreRef core) override;

  bool Start() override;

  void Shutdown() override;

 private:
  auto GetLogger() { return core_.GetLogger(); }

  aimrt::co::Task<void> GraspTask();

 private:
  aimrt::CoreRef core_;
  std::shared_ptr<aimrt::context::Context> ctx_ptr_;
  aimrt::co::AsyncScope scope_;

  aimrt::executor::ExecutorRef work_executor_;
  aimrt::context::res::Subscriber<PoseStamped> sub_flange_pose_;
  genisys_msgs::srv::IKCheckCoClient client_ik_check_;
  vision_grasp::graspnet_candidate::GraspCandidateServiceCoClient client_grasp_cand_;

  std::string topic_flange_pose_{};
  std::string work_executor_name_{};
  double rpc_frq_;

  std::optional<PoseStamped> current_flange_pose_;
  std::mutex flange_pose_mutex_;
};

}  // namespace genisys_grasp_decision::static_grasp_module
