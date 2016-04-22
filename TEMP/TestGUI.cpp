#include "TestGUI.h"
#include "utils\Log.h"
#include "imgui\IMGUI.h"

const char* ITEMS[] = { "item1", "item2", "item3", "item4", "item5", "item6", "item7" };

TestGUIState::TestGUIState() : ds::GameState("TestGUIState") {
	_dropDownItems.push_back("Entry 1");
	_dropDownItems.push_back("Entry 2");
	_dropDownItems.push_back("Entry 3");
	_dropDownItems.push_back("Entry 4");
	_dropDownItems.push_back("Entry 5");
	_dropDownItems.push_back("Entry 6");
	_dropDownItems.push_back("Entry 7");
	_dropDownItems.push_back("Entry 8");
	_dropdownState = 0;
	_dropdownSelection = -1;
	_check = false;
	_stepValue = 5.0f;
	_color = ds::Color(128, 64, 192, 48);
	_colorState = 0;

	for (int i = 0; i < 32; ++i) {
		_sinValues[i] = sin((float)i / 32.0f * TWO_PI);
	}

	_actions.push_back("Action 1");
	_actions.push_back("Action 2");
	_actions.push_back("Action 3");
	_actions.push_back("Action 4");
	_actions.push_back("Action 5");
}


TestGUIState::~TestGUIState() {
}

// --------------------------------------------
// activate
// --------------------------------------------
void TestGUIState::activate() {
	
}

// --------------------------------------------
// activate
// --------------------------------------------
void TestGUIState::deactivate() {
}

// --------------------------------------------
// update
// --------------------------------------------
int TestGUIState::update(float dt) {
	// nothing to do
	return 0;
}

// --------------------------------------------
// click
// --------------------------------------------
//int TestGUIState::onGUIButton(ds::DialogID dlgID, int button) {	
	//return button;
//}

// --------------------------------------------
// render
// --------------------------------------------
void TestGUIState::render() {
	v2 p = v2(300, 600);
	float values[] = { 0.2f, 0.4f, 0.6f, 0.3f, 0.5f, 0.9f, 0.8f };
	int state = 1;
	float f = 100.0f;
	int i = 123;
	v2 v2(500, 500);
	ds::Color clr(128, 32, 200, 192);
	gui::PushSetting(gui::GS_LABELSIZE,100.0f);
	gui::PushSetting(gui::GS_LINE_HEIGHT, 22.0f);
	//gui::PushSetting(gui::GS_ALPHA, 0.2f);
	gui::start(1, &p);
	gui::begin("Test",&state);
	gui::Button("Button");
	gui::Text("This is %d and %3.2f", i, f);
	gui::InputFloat("Float", &f);
	gui::InputInt("Int", &i);
	gui::InputVec2("Vec2", &v2);	
	gui::Separator();	
	gui::Label("Label", "%g %g", v2.x, v2.y);
	gui::Value("Value", v2);
	gui::InputColor("Color", &clr);
	gui::DropDownBox(ITEMS, 7, &_dropdownSelection, &_dropdownState);
	gui::CheckBox("Checkbox", &_check);
	gui::Slider("StepFloat", &_stepValue,0.0f, 10.0f,1.0f);	
	gui::Image("Sprite", ds::Rect(680, 0, 40, 40), 0);	
	gui::ColorSlider("Clr-Slider", &_color, &_colorState);
	gui::Diagram("Sin", _sinValues, 32, -1.0f, 1.0f, 0.1f);
	gui::ProgressBar("Progress", 75.0f);
	int selectedAction = gui::ActionBar(_actions);
	if (selectedAction != -1) {
		LOG << "selected action: " << selectedAction;
	}
	gui::PopSetting(gui::GS_LABELSIZE);
	gui::end();	
	gui::endFrame();
}


