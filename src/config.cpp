#include "config.h"
#include "message.pb.h"
#include "settings.pb.h"
#include "utilities.h"

namespace unifly {

    Config& Config::GetInstance()
	{
		static auto&& config = Config();
		return config;
	}

	void Config::LoadConfig(const unifly::schema::v1::Settings& settings)
	{
	    LOG_MSG("load config");
	    SetShowHideLabels(settings.labels_show());
	}

	void Config::SaveConfig(UniFly* instance)
	{
	    unifly::schema::v1::XPLMMessage settings_message;
		unifly::schema::v1::Settings* settings = settings_message.mutable_settings();
		settings->set_labels_show(GetShowHideLabels());

		instance->send_msg(settings_message);
	}

}
