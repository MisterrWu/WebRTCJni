/*
 *  Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/experiments/rate_control_settings.h"

#include "api/video_codecs/video_codec.h"
#include "api/video_codecs/video_encoder_config.h"
#include "test/field_trial.h"
#include "test/gtest.h"

namespace webrtc {

namespace {

TEST(RateControlSettingsTest, CongestionWindow) {
  EXPECT_FALSE(
      RateControlSettings::ParseFromFieldTrials().UseCongestionWindow());

  test::ScopedFieldTrials field_trials(
      "WebRTC-CongestionWindow/QueueSize:100/");
  const RateControlSettings settings_after =
      RateControlSettings::ParseFromFieldTrials();
  EXPECT_TRUE(settings_after.UseCongestionWindow());
  EXPECT_EQ(settings_after.GetCongestionWindowAdditionalTimeMs(), 100);
}

TEST(RateControlSettingsTest, CongestionWindowPushback) {
  EXPECT_FALSE(RateControlSettings::ParseFromFieldTrials()
                   .UseCongestionWindowPushback());

  test::ScopedFieldTrials field_trials(
      "WebRTC-CongestionWindow/QueueSize:100,MinBitrate:100000/");
  const RateControlSettings settings_after =
      RateControlSettings::ParseFromFieldTrials();
  EXPECT_TRUE(settings_after.UseCongestionWindowPushback());
  EXPECT_EQ(settings_after.CongestionWindowMinPushbackTargetBitrateBps(),
            100000u);
}

TEST(RateControlSettingsTest, PacingFactor) {
  EXPECT_FALSE(RateControlSettings::ParseFromFieldTrials().GetPacingFactor());

  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/pacing_factor:1.2/");
  const RateControlSettings settings_after =
      RateControlSettings::ParseFromFieldTrials();
  // Need to explicitly dereference the absl::optional
  // for the EXPECT_DOUBLE_EQ to compile.
  ASSERT_TRUE(settings_after.GetPacingFactor());
  EXPECT_DOUBLE_EQ(*settings_after.GetPacingFactor(), 1.2);
}

TEST(RateControlSettingsTest, AlrProbing) {
  EXPECT_FALSE(RateControlSettings::ParseFromFieldTrials().UseAlrProbing());

  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/alr_probing:1/");
  EXPECT_TRUE(RateControlSettings::ParseFromFieldTrials().UseAlrProbing());
}

TEST(RateControlSettingsTest, LibvpxVp8QpMax) {
  EXPECT_FALSE(RateControlSettings::ParseFromFieldTrials().LibvpxVp8QpMax());

  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/vp8_qp_max:50/");
  EXPECT_EQ(RateControlSettings::ParseFromFieldTrials().LibvpxVp8QpMax(), 50);
}

TEST(RateControlSettingsTest, DoesNotGetTooLargeLibvpxVp8QpMaxValue) {
  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/vp8_qp_max:70/");
  EXPECT_FALSE(RateControlSettings::ParseFromFieldTrials().LibvpxVp8QpMax());
}

TEST(RateControlSettingsTest, LibvpxVp8MinPixels) {
  EXPECT_FALSE(
      RateControlSettings::ParseFromFieldTrials().LibvpxVp8MinPixels());

  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/vp8_min_pixels:50000/");
  EXPECT_EQ(RateControlSettings::ParseFromFieldTrials().LibvpxVp8MinPixels(),
            50000);
}

TEST(RateControlSettingsTest, DoesNotGetTooSmallLibvpxVp8MinPixelValue) {
  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/vp8_min_pixels:0/");
  EXPECT_FALSE(
      RateControlSettings::ParseFromFieldTrials().LibvpxVp8MinPixels());
}

TEST(RateControlSettingsTest, LibvpxTrustedRateController) {
  const RateControlSettings settings_before =
      RateControlSettings::ParseFromFieldTrials();
  EXPECT_FALSE(settings_before.LibvpxVp8TrustedRateController());
  EXPECT_FALSE(settings_before.LibvpxVp9TrustedRateController());

  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/trust_vp8:1,trust_vp9:1/");
  const RateControlSettings settings_after =
      RateControlSettings::ParseFromFieldTrials();
  EXPECT_TRUE(settings_after.LibvpxVp8TrustedRateController());
  EXPECT_TRUE(settings_after.LibvpxVp9TrustedRateController());
}

TEST(RateControlSettingsTest, GetSimulcastHysteresisFactor) {
  const RateControlSettings settings_before =
      RateControlSettings::ParseFromFieldTrials();
  EXPECT_DOUBLE_EQ(settings_before.GetSimulcastHysteresisFactor(
                       VideoCodecMode::kRealtimeVideo),
                   1.0);
  EXPECT_DOUBLE_EQ(settings_before.GetSimulcastHysteresisFactor(
                       VideoEncoderConfig::ContentType::kRealtimeVideo),
                   1.0);
  EXPECT_DOUBLE_EQ(settings_before.GetSimulcastHysteresisFactor(
                       VideoCodecMode::kScreensharing),
                   1.35);
  EXPECT_DOUBLE_EQ(settings_before.GetSimulcastHysteresisFactor(
                       VideoEncoderConfig::ContentType::kScreen),
                   1.35);

  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/"
      "video_hysteresis:1.2,screenshare_hysteresis:1.4/");
  const RateControlSettings settings_after =
      RateControlSettings::ParseFromFieldTrials();

  EXPECT_DOUBLE_EQ(settings_after.GetSimulcastHysteresisFactor(
                       VideoCodecMode::kRealtimeVideo),
                   1.2);
  EXPECT_DOUBLE_EQ(settings_after.GetSimulcastHysteresisFactor(
                       VideoEncoderConfig::ContentType::kRealtimeVideo),
                   1.2);
  EXPECT_DOUBLE_EQ(settings_after.GetSimulcastHysteresisFactor(
                       VideoCodecMode::kScreensharing),
                   1.4);
  EXPECT_DOUBLE_EQ(settings_after.GetSimulcastHysteresisFactor(
                       VideoEncoderConfig::ContentType::kScreen),
                   1.4);
}

TEST(RateControlSettingsTest, TriggerProbeOnMaxAllocatedBitrateChange) {
  EXPECT_TRUE(RateControlSettings::ParseFromFieldTrials()
                  .TriggerProbeOnMaxAllocatedBitrateChange());

  test::ScopedFieldTrials field_trials(
      "WebRTC-VideoRateControl/probe_max_allocation:0/");
  EXPECT_FALSE(RateControlSettings::ParseFromFieldTrials()
                   .TriggerProbeOnMaxAllocatedBitrateChange());
}

TEST(RateControlSettingsTest, UseEncoderBitrateAdjuster) {
  // Should be off by default.
  EXPECT_FALSE(
      RateControlSettings::ParseFromFieldTrials().UseEncoderBitrateAdjuster());

  {
    // Can be turned on via field trial.
    test::ScopedFieldTrials field_trials(
        "WebRTC-VideoRateControl/bitrate_adjuster:true/");
    EXPECT_TRUE(RateControlSettings::ParseFromFieldTrials()
                    .UseEncoderBitrateAdjuster());
  }
}

}  // namespace

}  // namespace webrtc
