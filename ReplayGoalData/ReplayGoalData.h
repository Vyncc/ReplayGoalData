#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "RenderingTools.h"


#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

using namespace nlohmann;

class ReplayGoalData: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow/*, public BakkesMod::Plugin::PluginWindow*/
{
public:
	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	std::shared_ptr<CPPRP::ReplayFile> replayFile;

	std::vector<CPPRP::ReplicatedRBState> BallRBStates;
	std::vector<Vector> GoalHitLocations;
	std::vector<Vector> _shotLocations; ///< Stores the locations of goals/bounces of the current training pack.

	void GetGoalsData(std::filesystem::path filePath);

	void registerImpactLocation(Vector ballLocation);
	void renderOneFrame(CanvasWrapper canvas);
	LinearColor getHeatmapColor(float numberOfHitsInBracket);
	void drawRectangle(int x, int z, CanvasWrapper& canvas);
	void renderHeatMap(CanvasWrapper& canvas);
	bool _heatMapIsVisible = false;
	static const int XBrackets = 160; ///< Defines the number of brackets in X dimension. The number 8000 should be dividable by this number.
	static const int ZBrackets = 80; ///< Defines the number of brackets in Z dimension. The number 4000 should be dividable by this number.
	std::array<std::array<float, ZBrackets>, XBrackets> _heatmapData; ///< Stores the number of hits in each cell
	float _maximumValue = 0; ///< The maximum value of all brackets
	

	void startProcess();

	template<typename T>
	void try_get_prop(const std::string& prop_name, T& val)
	{
		try
		{
			val = replayFile->GetProperty<T>(prop_name);
		}
		catch (...) {}
	};

	template<typename T>
	void try_get_map_value(const std::string& prop_name, T& out_val, CPPRP::PropertyObj& obj)
	{
		if (const auto it = obj.find(prop_name); it != obj.end())
		{
			const auto& [property_name, property_type, value] = *it->second;
			if (std::holds_alternative<T>(value))
			{
				out_val = std::get<T>(value);
			}
		}
	};

	// Inherited via PluginSettingsWindow
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
	

	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "ReplayGoalData";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
};

