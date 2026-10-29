#include "static_grasp_module/static_grasp_module.h"

#include <chrono>
#include "yaml-cpp/yaml.h"

namespace genisys_grasp_decision::static_grasp_module {

bool StaticGraspModule::Initialize(aimrt::CoreRef core) {
  // Save aimrt framework handle
  core_ = core;
  ctx_ptr_ = std::make_shared<aimrt::context::Context>(core);

  try {
    // Read cfg
    auto file_path = ctx_ptr_->GetConfigFilePath();
    if (!file_path.empty()) {
      YAML::Node cfg_node = YAML::LoadFile(std::string(file_path));
      topic_flange_pose_ = cfg_node["topic_flange_pose"].as<std::string>();
      work_executor_name_ = cfg_node["work_executor_name"].as<std::string>();
      rpc_frq_ = cfg_node["rpc_frq"].as<double>();
    }

    //
    work_executor_ = ctx_ptr_->CreateExecutor(work_executor_name_);

    sub_flange_pose_ = ctx_ptr_->CreateSubscriber<PoseStamped>(
        topic_flange_pose_,
        [this](std::shared_ptr<const PoseStamped> msg) {
          std::lock_guard<std::mutex> lock(flange_pose_mutex_);
          current_flange_pose_ = *msg;
        });

    client_ik_check_ = ctx_ptr_->CreateClient<genisys_msgs::srv::IKCheckCoClient>();
    client_grasp_cand_ = ctx_ptr_->CreateClient<vision_grasp::graspnet_candidate::GraspCandidateServiceCoClient>();

  } catch (const std::exception& e) {
    AIMRT_ERROR("Init failed, {}", e.what());
    return false;
  }

  AIMRT_INFO("Init succeeded.");

  return true;
}

bool StaticGraspModule::Start() {
  auto scheduler = aimrt::co::AimRTScheduler(work_executor_);
  scope_.spawn(aimrt::co::On(scheduler, GraspTask()));
  AIMRT_INFO("static_grasp_module Started succeeded.");
  return true;
}

void StaticGraspModule::Shutdown() {
  ctx_ptr_->StopRunning();
  AIMRT_INFO("static_grasp_module Shutdown succeeded.");
}

aimrt::co::Task<void> StaticGraspModule::GraspTask() {
  ctx_ptr_->LetMe();

  double period_ms = 1000.0 / rpc_frq_;
  auto period = std::chrono::milliseconds(static_cast<uint32_t>(period_ms));
  try {
    AIMRT_INFO("Start RPC loop.");
    while (ctx_ptr_->Running()) {
      vision_grasp::graspnet_candidate::GraspCandidateRequest req;
      vision_grasp::graspnet_candidate::GraspCandidateResponse rsp;

      req.set_target_class("cuboid");
      req.set_target_class("red");
      req.set_max_candidates(32);
      req.set_build_o3d(true);
      req.set_detect_collision(true);

      auto status = co_await client_grasp_cand_.GetGraspCandidates(req, rsp);

      if (status.OK()) {
        AIMRT_INFO("RPC call succeeded, response: {}", rsp.candidates_size());
      } else {
        AIMRT_WARN("RPC call failed, status: {}", status.ToString());
      }

      auto scheduler = aimrt::co::AimRTScheduler(work_executor_);
      co_await aimrt::co::ScheduleAfter(scheduler, std::chrono::milliseconds(static_cast<uint32_t>(period_ms)));
    }
  } catch (const std::exception& e) {
    AIMRT_ERROR("GraspTask failed: {}", e.what());
  }
  AIMRT_INFO("StaticGraspModule GraspTask exit succeeded.");
  co_return;
}

}  // namespace genisys_grasp_decision::static_grasp_module
