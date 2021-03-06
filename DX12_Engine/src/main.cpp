// main definition
// create the engine and run it
#include "engine/Engine.h"
// engine game class
#include "engine/World.h"
#include "engine/Actor.h"
// WinMain
#include <Windows.h>

int WINAPI WinMain(
	HINSTANCE hInstance,    //Main windows function
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	Engine::EngineDesc desc;

	// define the engine setup
	desc.HInstance				= hInstance;
	desc.FramePerSecondTargeted = 60;
	desc.CameraPosition			= XMFLOAT4(0.f, 1.f, 5.f, 0.f);

	// create the engine singleton
	Engine::Create();
	Engine & engine = Engine::GetInstance();

	// initialize and run the engine
	engine.Initialize(desc);

	// initialize the world
	World * world = engine.GetWorld();

	// create objects...
	Actor::ActorDesc planeActor;
	planeActor.Name = L"Plane";
	planeActor.Mesh = "Primitive:Plane";
	world->SpawnActor(planeActor, Transform(XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(-90.f, 0.f, 0.f), XMFLOAT3(10.f, 10.f, 1.f)));

	Actor::ActorDesc lightActor;
	lightActor.Name		= L"Light";
	lightActor.IsLight	= true;
	world->SpawnActor(lightActor, Transform(XMFLOAT3(0.f, 0.5f, 0.f)));

	engine.Run();
}