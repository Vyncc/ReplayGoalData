#include "pch.h"
#include "ReplayGoalData.h"

std::string ReplayGoalData::GetPluginName() 
{
	return "ReplayGoalData";
}

void ReplayGoalData::SetImGuiContext(uintptr_t ctx) 
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> ReplayGoalData
void ReplayGoalData::RenderSettings()
{
	ImGui::Checkbox("_heatMapIsVisible", &_heatMapIsVisible);
	ImGui::Checkbox("_impactMapIsVisible", &_impactMapIsVisible);

	for (auto goalHitLoc : GoalHitLocations)
	{
		ImGui::Text("X : %f | Y : %f | Z : %f", goalHitLoc.X, goalHitLoc.Y, goalHitLoc.Z);
	}
}


/*
// Do ImGui rendering here
void ReplayGoalData::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string ReplayGoalData::GetMenuName()
{
	return "ReplayGoalData";
}

// Title to give the menu
std::string ReplayGoalData::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void ReplayGoalData::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool ReplayGoalData::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool ReplayGoalData::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void ReplayGoalData::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void ReplayGoalData::OnClose()
{
	isWindowOpen_ = false;
}
*/
