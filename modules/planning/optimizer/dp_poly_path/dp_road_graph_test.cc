/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ros/include/ros/ros.h"

#include "modules/planning/proto/dp_poly_path_config.pb.h"

#include "modules/common/adapters/adapter_gflags.h"
#include "modules/common/adapters/adapter_manager.h"
#include "modules/common/log.h"
#include "modules/common/util/file.h"
#include "modules/planning/common/data_center.h"
#include "modules/planning/common/planning_gflags.h"
#include "modules/planning/optimizer/dp_poly_path/dp_road_graph.h"
#include "modules/planning/optimizer/dp_poly_path/path_sampler.h"
#include "modules/planning/optimizer/optimizer_test_base.h"

namespace apollo {
namespace planning {

using common::adapter::AdapterManager;

class DpRoadGraphTest : public OptimizerTestBase {
 public:
  void SetInitPointWithVelocity(const double velocity) {
    init_point_.mutable_path_point()->set_x(586392.84003);
    init_point_.mutable_path_point()->set_y(4140673.01232);
    init_point_.set_v(velocity);
    init_point_.set_a(0.0);
    init_point_.set_relative_time(0.0);
  }

  void SetSpeedDataWithConstVeolocity(const double velocity) {
    // speed point params: s, time, v, a, da
    double t = 0.0;
    const double delta_s = 1.0;
    if (velocity > 0.0) {
      for (double s = 0.0; s < 100.0; s += delta_s) {
        speed_data_.add_speed_point(s, t, velocity, 0.0, 0.0);
        t += delta_s / velocity;
      }
    } else {  // when velocity = 0, stand still
      for (double t = 0.0; t < 10.0; t += 0.1) {
        speed_data_.add_speed_point(0.0, t, 0.0, 0.0, 0.0);
      }
    }
  }

  virtual void SetUp() {
    google::InitGoogleLogging("DpRoadGraphTest");
    OptimizerTestBase::SetUp();
    reference_line_ = &(frame_->planning_data().reference_line());
  }

 protected:
  const ReferenceLine* reference_line_ = nullptr;
  common::TrajectoryPoint init_point_;
  DecisionData decision_data_;
  SpeedData speed_data_;  // input
  PathData path_data_;    // output
};

TEST_F(DpRoadGraphTest, speed_road_graph) {
  SetInitPointWithVelocity(10.0);
  SetSpeedDataWithConstVeolocity(10.0);

  DpRoadGraph road_graph(dp_poly_path_config_, init_point_, speed_data_);
  ASSERT_TRUE(reference_line_ != nullptr);
  bool result =
      road_graph.find_tunnel(*reference_line_, &decision_data_, &path_data_);
  EXPECT_TRUE(result);
  EXPECT_EQ(706, path_data_.discretized_path().num_of_points());
  EXPECT_EQ(706, path_data_.frenet_frame_path().number_of_points());
  EXPECT_FLOAT_EQ(69.9, path_data_.frenet_frame_path().points().back().s());
  EXPECT_FLOAT_EQ(0.0, path_data_.frenet_frame_path().points().back().l());
  // export_path_data(path_data_, "/tmp/path.txt");
}

}  // namespace planning
}  // namespace apollo
