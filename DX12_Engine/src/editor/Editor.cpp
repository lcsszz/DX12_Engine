#include "Editor.h"

// editors windows
#include "UIMaterialBuilder.h"
#include "UISceneBuilder.h"
#include "UIActorBuilder.h"
// other
#include "ui/UILayer.h"
#include "engine/Engine.h"

void Editor::SetActive(bool i_Active)
{
	if (i_Active && !m_IsEnabled)		OpenEditor();
	else if (!i_Active && m_IsEnabled)	CloseEditor();

	m_IsEnabled = i_Active;
}

bool Editor::IsActive() const
{
	return m_IsEnabled;
}

Editor::Editor(const EditorDesc & i_Desc)
{
	m_IsEnabled = false;

	// generate the windows
	m_MaterialBuilder = new UIMaterialBuilder;
	m_ActorBuilder = new UIActorBuilder;
	m_SceneBuilder = new UISceneBuilder(m_ActorBuilder);

	m_Shortcuts.InputShowMaterial	= i_Desc.InputShowMaterial;
	m_Shortcuts.InputShowScene		= i_Desc.InputShowScene;
	m_Shortcuts.InputActorBuilder	= i_Desc.InputActorBuilder;

	m_Layer = i_Desc.Layer;

	// open (or not) the editor
	SetActive(i_Desc.EnabledByDefault);

	// generate windows data
	// scene editor
	m_SceneBuilder->SetWorld(Engine::GetInstance().GetWorld());
}

Editor::~Editor()
{
	// be sure that events are ubinded
	UnbindKeyboardEvents();
	CloseEditor();
}

FORCEINLINE void Editor::BindKeyboardEvents()
{
	// generate inputs
	Input::BindKeyEvent(Input::eKeyDown, m_Shortcuts.InputShowScene.KeyCode, "EditorShowSceneBuilder", m_Shortcuts.InputShowScene.KeyFlags, this, &Editor::SetSceneBuilderShow, nullptr);
	Input::BindKeyEvent(Input::eKeyDown, m_Shortcuts.InputActorBuilder.KeyCode, "EditorShowActorBuilder", m_Shortcuts.InputActorBuilder.KeyFlags, this, &Editor::SetSceneBuilderShow, nullptr);
}

FORCEINLINE void Editor::UnbindKeyboardEvents()
{
	Input::UnbindKeyEvent(Input::eKeyDown, m_Shortcuts.InputShowScene.KeyCode, "EditorShowSceneBuilder");
	Input::UnbindKeyEvent(Input::eKeyDown, m_Shortcuts.InputActorBuilder.KeyCode, "EditorShowActorBuilder");
}

void Editor::CloseEditor()
{
	ASSERT(m_IsEnabled == true);

	// popup the windows from the layer
	m_Layer->PopUIWindowFromLayer(m_SceneBuilder);
	m_Layer->PopUIWindowFromLayer(m_ActorBuilder);

	UnbindKeyboardEvents();

	m_IsEnabled = false;
}

void Editor::OpenEditor()
{
	ASSERT(m_IsEnabled == false);

	// push windows on the layer
	m_Layer->PushUIWindowOnLayer(m_SceneBuilder);
	m_Layer->PushUIWindowOnLayer(m_ActorBuilder);
	
	BindKeyboardEvents();

	m_IsEnabled = true;
}

void Editor::SetMaterialBuilderShow(void * i_Void)
{
	m_MaterialBuilder->SetActive(!m_MaterialBuilder->IsActive());
}

void Editor::SetSceneBuilderShow(void * i_Void)
{
	m_SceneBuilder->SetActive(!m_SceneBuilder->IsActive());
}

void Editor::SetEditorShow(void * i_Void)
{
	if (m_IsEnabled)	CloseEditor();
	else				OpenEditor();
}