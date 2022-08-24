#include "pch.h"
#include "ReplayGoalData.h"

#include <filesystem>
#include <thread>

BAKKESMOD_PLUGIN(ReplayGoalData, "ReplayGoalData", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

const auto YDrawLocation = 5100.0f;
const auto XBracketWidth = 8000 / ReplayGoalData::XBrackets;
const auto ZBracketHeight = 4000 / ReplayGoalData::ZBrackets;

void ReplayGoalData::onLoad()
{
	_globalCvarManager = cvarManager;

	gameWrapper->RegisterDrawable(std::bind(&ReplayGoalData::renderOneFrame, this, std::placeholders::_1));
	
	cvarManager->registerNotifier("GetGoalsData", [&](std::vector<std::string> args)
		{


			std::thread t2(&ReplayGoalData::startProcess, this);
			t2.detach();

		}, "", 0);
}

void ReplayGoalData::startProcess()
{
	GoalHitLocations.clear();
	_shotLocations.clear();

	std::string replaysPath = "C:\\Users\\snipj\\Documents\\replaysTest";
	for (const auto& file : std::filesystem::directory_iterator(replaysPath))
	{
		LOG("Getting goals data from {}", file.path().string());
		GetGoalsData(file.path());
	}

	LOG("GoalHitLocations count : {}", GoalHitLocations.size());


	//for (auto goalHitLoc : GoalHitLocations)
	//{
	//	registerImpactLocation(goalHitLoc);
	//}
	//LOG("Registration finished!");
}

void ReplayGoalData::GetGoalsData(std::filesystem::path filePath)
{
	replayFile = std::make_shared<CPPRP::ReplayFile>(filePath.string());

	replayFile->Load();
	replayFile->DeserializeHeader();

	BallRBStates.clear();

	replayFile->tickables.push_back([&](const CPPRP::Frame f, const std::unordered_map<uint32_t, CPPRP::ActorStateData>& actorStats)
		{
			for (auto& actor : actorStats)
			{
				std::shared_ptr<CPPRP::TAGame::Ball_TA> ball = std::dynamic_pointer_cast<CPPRP::TAGame::Ball_TA>(actor.second.actorObject);
				if (ball)
				{
					CPPRP::ReplicatedRBState ballrbstate = ball->ReplicatedRBState;
					BallRBStates.push_back(ballrbstate);
				}
			}
		});



	replayFile->Parse();


	//const std::string replay_name = static_cast<std::string>(replayFile->GetProperty<std::string>("ReplayName")); //working too
	auto replay_name = replayFile->GetProperty<std::string>("ReplayName");
	auto replay_matchType = replayFile->GetProperty<std::string>("MatchType");

	//replayFile->pro

	/*cvarManager->log("replay name : " + replay_name);
	cvarManager->log("replay matchType : " + replay_matchType);
	LOG("frames count : {}", replayFile->frames.size());
	LOG("BallRBStates count : {}", BallRBStates.size());*/

	/*for (auto prop : replayFile->replayFile->properties)
	{
		LOG("first : {}", prop.first);
		LOG("second : property_name : {}", prop.second->property_name);
		LOG("second : property_name : {}", prop.second->property_type);
		LOG("-----------------------------------------------------------");
	}*/


	std::vector<CPPRP::PropertyObj> goals;
	try_get_prop("Goals", goals);
	/*LOG("Goals count : {}", goals.size());*/
	int goal_index = 0;
	for (auto goal : goals)
	{
		int32_t frame;
		try_get_map_value("frame", frame, goal);

		/*LOG("--------------------------------------------");
		LOG("Goal {} :", goal_index);
		LOG("frame {}", frame);
		LOG("Ball Location : x : {} | y : {} | z : {}", BallRBStates[frame].position.x, BallRBStates[frame].position.y, BallRBStates[frame].position.z);
		LOG("--------------------------------------------");*/

		Vector GoalHitLoc = { BallRBStates[frame].position.x, BallRBStates[frame].position.y, BallRBStates[frame].position.z };
		if (GoalHitLoc.Y < 0) //if the goal is blue side
		{
			GoalHitLoc.X = GoalHitLoc.X * -1;
		}
		GoalHitLocations.push_back(GoalHitLoc);
		registerImpactLocation(GoalHitLoc);

		goal_index++;
	}
}

void ReplayGoalData::registerImpactLocation(Vector ballLocation)
{
	_shotLocations.emplace_back(Vector(ballLocation.X, YDrawLocation - 10.0f, ballLocation.Z));

	// Get the array bracket for the X dimension
	auto xBracket = (int)(ballLocation.X + 4000) / XBracketWidth;
	// Get the array bracket for the Z dimension
	auto zBracket = (int)ballLocation.Z / ZBracketHeight;

	for (auto x = xBracket - 5; x < XBrackets && x <= xBracket + 5; x++)
	{
		auto xDifference = abs((float)(x - xBracket));
		auto xMagnitude = 0.5f / pow(2.0f, xDifference);

		for (auto z = zBracket - 5; z < ZBrackets && z <= zBracket + 5; z++)
		{
			auto zDifference = abs((float)(z - zBracket));
			auto zMagnitude = 0.5f / pow(2.0f, zDifference);

			if (x >= 0 && z >= 0)
			{
				auto& newValue = _heatmapData[x][z];
				newValue += (xMagnitude + zMagnitude);
				if (newValue > _maximumValue)
				{
					_maximumValue = newValue;
				}
			}
		}
	}
}

void drawRect(float left, float bottom, float width, float height, float y, const CameraWrapper& camera, CanvasWrapper& canvas)
{
	auto bottomLeft = Vector{ left, y, bottom };
	auto topRight = Vector{ left + width, y, bottom + height };

	// Skip drawing if either of the points is not in the currently visible area
	auto currentCameraFrustum = RT::Frustum(canvas, camera);
	if (!currentCameraFrustum.IsInFrustum(bottomLeft) || !currentCameraFrustum.IsInFrustum(topRight))
	{
		return;
	}
	auto bottomLeftProj = canvas.Project(bottomLeft);
	auto topRightProj = canvas.Project(topRight);

	// Unfortunately we can't use triangles since these will ignore the alpha channel
	canvas.DrawRect(bottomLeftProj, topRightProj);
}

void ReplayGoalData::renderOneFrame(CanvasWrapper canvas)
{
	canvas.SetPosition(Vector2{});

	if (_heatMapIsVisible)
	{
		renderHeatMap(canvas);
	}
}


void ReplayGoalData::renderHeatMap(CanvasWrapper& canvas)
{
	for (auto x = 0; x < XBrackets; x++)
	{
		for (auto z = 0; z < ZBrackets; z++)
		{
			auto numberOfHitsInBracket = _heatmapData[x][z];

			// If there are zero hits, don't paint it
			if (numberOfHitsInBracket > 0)
			{
				canvas.SetColor(getHeatmapColor(numberOfHitsInBracket));
				drawRectangle(x, z, canvas);
			}
		}
	}
}

void ReplayGoalData::drawRectangle(int x, int z, CanvasWrapper& canvas)
{
	auto leftBorder = (float)(x * XBracketWidth - 4000);
	auto bottomBorder = (float)z * ZBracketHeight;

	drawRect(leftBorder, bottomBorder, XBracketWidth, ZBracketHeight, YDrawLocation, gameWrapper->GetCamera(), canvas);
}

LinearColor ReplayGoalData::getHeatmapColor(float numberOfHitsInBracket)
{
	// calculate the color like this:
	// 0% of max value: black
	// 20% of max value: pure Blue
	// 40% of max value: blue + green (cyan)
	// 60% of max value: pure green
	// 80% of max value: green + red (yellow)
	// 100% of max value: pure red

	auto percentageOfMaximum = numberOfHitsInBracket / _maximumValue;

	// Red: 0%-60% = 0, 80%-100% = 1
	// Green: 0-20% = 0, 40-60% = 1, 80-100% = 0
	// Blue: 0% = 0, 20-40% = 1, 60-100% = 0
	float redFactor, greenFactor, blueFactor;
	if (percentageOfMaximum <= .2f)
	{
		redFactor = .0f;
		greenFactor = .0f;
		blueFactor = percentageOfMaximum * 5.0f; // Scale to 0-100
	}
	else if (percentageOfMaximum <= .4f)
	{
		redFactor = .0f;
		greenFactor = (percentageOfMaximum - .2f) * 5.0f;
		blueFactor = 1.0f;
	}
	else if (percentageOfMaximum <= .6f)
	{
		redFactor = .0f;
		greenFactor = 1.0f;
		blueFactor = 1.0f - ((percentageOfMaximum - .4f) * 5.0f);
	}
	else if (percentageOfMaximum <= .8f)
	{
		redFactor = (percentageOfMaximum - .6f) * 5.0f;
		greenFactor = 1.0f;
		blueFactor = .0f;
	}
	else
	{
		redFactor = 1.0f;
		greenFactor = 1.0f - ((percentageOfMaximum - .8f) * 5.0f);
		blueFactor = .0f;
	}
	return LinearColor{ 255.0f * redFactor, 255.0f * greenFactor, 255.0f * blueFactor, 50.0f + percentageOfMaximum * 200.0f };
}

void ReplayGoalData::onUnload()
{
}