#pragma once
#include "metalstation.h"
#include "../common/config.h"
#include "../common/collectstation.h"
#include "../common/unloadstation.h"
#include "../common/mountstation.h"
#include "../../../core/istationmanager.h"

namespace llpp::bots::farm
{
    class MetalManager final : public core::IStationManager
    {
    public:
        explicit MetalManager(FarmConfig* t_config);

        bool run() override;

        [[nodiscard]] bool is_ready_to_run() override;

        [[nodiscard]] std::chrono::minutes get_time_left_until_ready() const override;
    private:
        FarmConfig* config_;

        std::shared_ptr<asa::entities::DinoEntity> anky_;

        core::TeleportStation start_tp_;
        core::TeleportStation load_tp_;

        UnloadStation unload_station_;
        CollectStation collect_station_;

        MountStation mount_station_;

        std::vector<std::unique_ptr<MetalStation>> stations_;
    };

    std::vector<std::unique_ptr<MetalManager>> create_metal_managers();
}
