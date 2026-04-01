#include "unifly.h"

namespace unifly {

class Config {
    public:
  		static Config& GetInstance();
		~Config() = default;
		Config(const Config&) = delete;
		void operator=(const Config&) = delete;
		Config(Config&&)noexcept = default;
		Config& operator=(Config&&)noexcept = default;


		void LoadConfig(const unifly::schema::v1::Settings& settings);
		void SaveConfig(UniFly* instance);

		void SetShowHideLabels(bool status) { m_showHideLabels = status; }
		bool GetShowHideLabels() const { return m_showHideLabels; }

	private:
		Config() = default;

		bool m_showHideLabels = true;



};

}
