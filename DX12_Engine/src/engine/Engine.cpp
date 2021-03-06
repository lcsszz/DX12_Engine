#include "Engine.h"

// dx12
#include "dx12/DX12Utils.h"
#include "dx12/DX12Context.h"
#include "dx12/DX12RenderEngine.h"
#include "dx12/DX12ConstantBuffer.h"
#include "dx12/DX12ImGui.h"
// game include
#include "engine/World.h"
#include "engine/Camera.h"
#include "components/RenderComponent.h"
// engine
#include "engine/Clock.h"
#include "engine/Window.h"
#include "engine/Console.h"
#include "engine/RenderList.h"
// resources
#include "resource/ResourceManager.h"		// CPU side resource (that can load also GPU resources)
#include "resource/DX12ResourceManager.h"	// GPU side resources
// ui
#include "ui/UILayer.h"
#include "ui/UIConsole.h"
#include "ui/UIDebug.h"
// editor
#include "editor/Editor.h"
// resources
#include "resource/DX12ResourceManager.h"
#include "resource/ResourceManager.h"


// debug purpose
#ifdef ENGINE_DEBUG
#include "resource/Material.h"
#endif

Engine *		Engine::s_Instance = nullptr;

Engine & Engine::GetInstance()
{
	ASSERT(s_Instance != nullptr);
	return *s_Instance;
}

void Engine::Create()
{
	ASSERT(s_Instance == nullptr);
	s_Instance = new Engine;
}

void Engine::Delete()
{
	ASSERT(s_Instance != nullptr);
	delete s_Instance;
	s_Instance = nullptr;
}

float Engine::GetLifeTime() const
{
	return m_EngineClock->GetElapsedFromStart().ToSeconds();
}

float Engine::GetFrameTime() const
{
	return m_ElapsedTime;
}

UINT Engine::GetFramePerSecond() const
{
	return m_FramePerSecond;
}

UINT Engine::GetFramePerSecondTarget() const
{
	return m_FramePerSecondsTargeted;
}

void Engine::SetFramePerSecondTarget(UINT i_Target)
{
	m_FramePerSecondsTargeted = i_Target;
}

void Engine::Initialize(EngineDesc & i_Desc)
{
	// create the DX12RenderEngine
	if (i_Desc.HInstance == nullptr)
	{
		PRINT_DEBUG("Error, hInstance not filled in Engine Desc");
		return;
	}

	// create the window
	m_Window = new Window(i_Desc.HInstance, i_Desc.WindowName.c_str(), i_Desc.WindowName.c_str(), i_Desc.WindowSize.x, i_Desc.WindowSize.y, i_Desc.WindowIcon);

	// retreive the render engine
	DX12RenderEngine::Create(i_Desc.HInstance);
	m_RenderEngine = &DX12RenderEngine::GetInstance();
	m_RenderEngine->InitializeDX12();

	// resource management
	m_ResourceManager			= new ResourceManager;
	m_RenderResourceManager		= new DX12ResourceManager;	// create GPU resources (need DX12Initialized)

	// initialize rendering pipeline and GBuffer creation (need the DX12ResourceManager)
	m_RenderEngine->InitializeRender();

	// intialize constant buffer
	m_RenderEngine->GetConstantBuffer(DX12RenderEngine::eGlobal)->ReserveVirtualAddress();	// reserve the first address on the constant buffer

	// create input management
	m_Window->RegisterInputCallback(&Input::ProcessInputCallbacks);

	World::WorldDesc worldDesc;
	// create default camera parameters
	worldDesc.CameraPosition		= i_Desc.CameraPosition;
	worldDesc.CameraTarget			= i_Desc.CameraTarget;
	worldDesc.UseCameraProjection	= i_Desc.UseCameraProjection;
	worldDesc.CameraProjection		= i_Desc.CameraProjection;

	// create game
	m_EngineClock = new Clock;
	m_CurrentWorld = new World(worldDesc);

	// create managers
	m_RenderList = new RenderList;

	// setup settings
	m_FramePerSecondsTargeted = i_Desc.FramePerSecondTargeted;
	m_ElapsedTime = 0.f;

	// initialize UI
	m_UILayer = new UILayer(m_Window);
	m_UILayer->SetEnable(i_Desc.UIEnabled);

#if  defined(_DEBUG) || defined(WITH_EDITOR)
	// ui dev initialization
	m_UIConsole = new UIConsole;
	m_UIDebug	= new UIDebug;

	// initialize console
	m_Console = new Console;
	m_Console->RegisterPrintCallback(UIConsole::StaticPrint, m_UIConsole);
	// push default command
	m_Console->RegisterFunction(new CFClear);
	m_Console->RegisterFunction(new CFActorCount);
	m_Console->RegisterFunction(new CFHelp);
	m_Console->RegisterFunction(new CFPrintParam);
	m_Console->RegisterFunction(new CFSetFrameTarget);

	// push windows on layer
	m_UILayer->PushUIWindowOnLayer(m_UIConsole);
	m_UILayer->PushUIWindowOnLayer(m_UIDebug);

	Input::BindKeyEvent<Engine>(Input::eKeyDown, VK_F1, "F1Debug", this, &Engine::OnF1Down, nullptr);
	Input::BindKeyEvent<Engine>(Input::eKeyDown, VK_F2, "F2Debug", this, &Engine::OnF2Down, nullptr);
	Input::BindKeyEvent<Engine>(Input::eKeyDown, VK_F3, "F3Debug", this, &Engine::OnF3Down, nullptr);

	m_IsInGame = false;

#ifdef WITH_EDITOR
	Editor::EditorDesc editorDesc;

	editorDesc.Layer = m_UILayer;
	editorDesc.EnabledByDefault = false;

	// editor creation
	m_Editor = new Editor(editorDesc);
	Input::BindKeyEvent<Engine>(Input::eKeyDown, VK_F5, "F5Editor", this, &Engine::OnF5Down, nullptr);
#endif

#endif

#ifdef _DEBUG
	PRINT_DEBUG("DX12 engine [version 0.1]");
	PRINT_DEBUG("Initilization... OK");
	PRINT_DEBUG("ImGui version %s", IMGUI_VERSION);
#endif

	// enable input
	Input::SetKeyEventEnabled(true);

#ifdef ENGINE_DEBUG
	Material::MaterialData matData;
	matData.Name = "Default";
	matData.Materials = new Material::MaterialSpec[1];
	matData.Materials[0].Ka = color::Pink;
	matData.Materials[0].Ke = color::Pink;
	matData.Materials[0].Ks = color::Pink;
	matData.Materials[0].Kd = color::Pink;
	matData.Materials[0].Ns = 32.f;
	matData.Materials[0].Name = "Default";
	matData.MaterialCount = 1;

	m_ResourceManager->LoadMaterialWithData(&matData);
#endif
	// preload the resources
	m_RenderResourceManager->PushResourceOnGPUWithWait();

	m_Exit = false;
}

void Engine::Run()
{
	// start engine clock
	m_EngineClock->Reset();

	// To do : make a command list and a push to GPU
	// Workaround : render before doing anything
	m_RenderEngine->Render();

	while (!m_Exit)
	{
		// load resources if needed
		m_RenderResourceManager->PushResourceOnGPUWithWait();	// Workaround : load each time a new objects

		// pre update management
		m_ElapsedTime = m_EngineClock->Restart().ToSeconds();

		// retreive performance data
		if (m_ElapsedTime != 0.f)
		{ 
			m_FramePerSecond = (UINT)(1.f / m_ElapsedTime);
		}

#if defined (_DEBUG) || defined(WITH_EDITOR)
		if (!m_IsInGame)
		{
			// camera management depending if the windows are selected or not
			Camera * const freeCam = m_CurrentWorld->GetCurrentCamera();
			freeCam->SetFreecamEnabled(!m_UILayer->IsOneWindowFocused());

			// Input management
			Input::SetKeyEventEnabled(!m_UILayer->IsOneWindowFocused());
		}

#endif
		/* -- Update -- */

		// update input and window callbacks
		m_Window->Update();

		// tick the world (update all actors and components)
		ASSERT(m_CurrentWorld != nullptr);
		if (m_IsInGame)
		{
			// here we update the game
			m_CurrentWorld->TickWorld(m_ElapsedTime);
		}
		else
		{
			m_CurrentWorld->TickCamera(m_ElapsedTime);
		}

		// update and prepare the ui
		{
			// this create vertices and data for the UI layer
			m_UILayer->DisplayUIOnLayer();
		}

		/* -- Render -- */

		// prepare the render engine
		m_RenderEngine->PrepareForRender();	// intialize the command list and other stuff
		ID3D12GraphicsCommandList * commandList = m_RenderEngine->GetContext(DX12RenderEngine::eImmediate)->GetCommandList();

		// update global buffer
		{
			struct GlobalBuffer
			{
				// other useful matrix for effects
				float		Time;		// application time (from engine initialization)
				float		Elapsed;	// frame time
				XMFLOAT4	CamPos;		// position of the camera
			};

			static GlobalBuffer buff = {};
			buff.Elapsed = m_ElapsedTime;
			buff.Time = m_EngineClock->GetElapsedFromStart().ToSeconds();
			buff.CamPos = m_CurrentWorld->GetCurrentCamera()->m_Position;

			// update constant buffer
			m_RenderEngine->GetConstantBuffer(DX12RenderEngine::eGlobal)->UpdateConstantBuffer(0, &buff, sizeof(GlobalBuffer));
		}

		// setup and push render list on the commandlist
		{
			RenderList::RenderListSetup setup;
			Camera * cam = m_CurrentWorld->GetCurrentCamera();

			// dx12 related
			setup.DeferredCommandList	= m_RenderEngine->GetContext(DX12RenderEngine::eDeferred)->GetCommandList();
			setup.ImmediateCommandList	= m_RenderEngine->GetContext(DX12RenderEngine::eImmediate)->GetCommandList();
			// camera related
			setup.ProjectionMatrix	= XMLoadFloat4x4(&cam->GetProjMatrix());
			setup.ViewMatrix		= XMLoadFloat4x4(&cam->GetViewMatrix());
			setup.CameraPosition	= XMFLOAT3(&m_CurrentWorld->GetCurrentCamera()->m_Position.x);

			// setup render list
			m_RenderList->Reset();	// reset the render list of the previous frame
			m_RenderList->SetupRenderList(setup);
			// push components to render to the render list
			m_CurrentWorld->RenderWorld(m_RenderList);
		}

		// render the GBuffer
		{
			// push the components on the commandlist to prepare for a render
			m_RenderList->RenderGBuffer();
		}

		// render lights in deferred
		{
#ifdef ENGINE_DEBUG
			// if we draw debug
			if (!m_RenderEngine->DebugIsEnabled())
			{
				// render lights
				m_RenderList->RenderLight();
			}
#else
			// render lights
			m_RenderList->RenderLight();

#endif /* ENGINE_DEBUG */
		}

		// render ui
		{
			m_UILayer->PushOnCommandList(commandList);
		}

		// update and display backbuffer, also swap buffer and manage commandqueue
		m_RenderEngine->Render();

		/* -- End of the loop -- */
		// update exit 
		if (!m_Window->IsOpen())
		{
			m_Exit = true;
		}

		// wait before the next loop if we are too fast
		if (m_FramePerSecondsTargeted != 0)
		{
			// sleep time
			float frameTime = m_EngineClock->GetElaspedTime().ToSeconds();
			const float fpsTime = (1.f / (float)m_FramePerSecondsTargeted);

			if (frameTime < fpsTime)
			{
				DWORD sleepTime = (DWORD)((fpsTime - frameTime) * 1000.f);
				Sleep(sleepTime);
			}
		}
	}

	// close the dx12 commandlist
	m_RenderEngine->Close();

	// exit the engine
	CleanUpResources();
	CleanUpModules();
}

Window * Engine::GetWindow() const
{
	return m_Window;
}

DX12ResourceManager * Engine::GetRenderResourceManager() const
{
	return m_RenderResourceManager;
}

ResourceManager * Engine::GetResourceManager() const
{
	return m_ResourceManager;
}

World * Engine::GetWorld() const
{
	return m_CurrentWorld;
}

UILayer * Engine::GetUILayer() const
{
	return m_UILayer;
}

Console * Engine::GetConsole() const
{
	return m_Console;
}

RenderList * Engine::GetRenderList() const
{
	return m_RenderList;
}

Engine::Engine()
	:m_RenderEngine(nullptr)
	,m_CurrentWorld(nullptr)
	,m_EngineClock(nullptr)
	,m_Window(nullptr)
	// managers
	,m_ResourceManager(nullptr)
	,m_RenderResourceManager(nullptr)
	,m_Console(nullptr)
	// ui
	,m_UILayer(nullptr)
	,m_UIConsole(nullptr)
	// setup
	,m_Exit(true)
{
}

Engine::~Engine()
{
}

void Engine::CleanUpResources()
{
	// clean resources
	m_ResourceManager->CleanResources();
}

void Engine::CleanUpModules()
{

	// delete UI
	delete m_UILayer;
	delete m_UIConsole;

	// delete manager
	delete m_Console;
	delete m_RenderResourceManager;
	delete m_ResourceManager;

	delete m_Window;

	// delete the render engine
	// To do : fix crash when releasing resources
	DX12RenderEngine::Delete();
}
 
#if defined(_DEBUG) || defined(WITH_EDITOR)
void Engine::OnF1Down(void * i_Void)
{
	m_UIConsole->SetActive(!m_UIConsole->IsActive());
}

void Engine::OnF2Down(void * i_Void)
{
	m_UIDebug->SetActive(!m_UIDebug->IsActive());
}

void Engine::OnF3Down(void * i_Void)
{
	m_RenderEngine->EnableDebug(!m_RenderEngine->DebugIsEnabled());
}

#ifdef WITH_EDITOR

void Engine::OnF5Down(void * i_Void)
{
	m_Editor->SetActive(!m_Editor->IsActive());
}

bool Engine::IsInGame() const
{
	return false;
}
#endif

#endif