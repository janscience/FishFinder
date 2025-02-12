#include <ScreenMenu.h>


#define BLACK    0x0000
#define WHITE    0xFFFF


ScreenMenu::ScreenMenu(Display *screen, PushButtons *buttons) :
  Buttons(buttons),
  UpID(-1),
  DownID(-1),
  SelectID(-1),
  BackID(-1),
  Screen(screen),
  CheckedAction(0),
  CheckedReturns(false),
  NActions(0),
  Title(""),
  Canvas(0),
  Baseline(0),
  ActionHeight(0),
  Index(0) {
  memset(Texts, 0, sizeof(Texts));
  memset(Actions, 0, sizeof(Actions));
  memset(Checked, 0, sizeof(Checked));
  memset(RadioButton, 0, sizeof(RadioButton));
  memset(Menus, 0, sizeof(Menus));
  memset(IDs, 0, sizeof(IDs));
  memset(YPos, 0, sizeof(YPos));
}


void ScreenMenu::setButtons(PushButtons *buttons, int up_id, int down_id,
		      int select_id, int back_id) {
  Buttons = buttons;
  UpID = up_id;
  DownID = down_id;
  SelectID = select_id;
  BackID = back_id;
  for (int k=0; k<NActions; k++) {
    if (Menus[k] != 0)
      Menus[k]->setButtons(buttons, up_id, down_id, select_id, back_id);
  }
}


void ScreenMenu::setButtons(int up_id, int down_id,
		      int select_id, int back_id) {
  UpID = up_id;
  DownID = down_id;
  SelectID = select_id;
  BackID = back_id;
  for (int k=0; k<NActions; k++) {
    if (Menus[k] != 0)
      Menus[k]->setButtons(up_id, down_id, select_id, back_id);
  }
}


void ScreenMenu::setDisplay(Display *screen) {
  Screen = screen;
  for (int k=0; k<NActions; k++) {
    if (Menus[k] != 0)
      Menus[k]->setDisplay(screen);
  }
}


void ScreenMenu::setTitle(const char *title) {
  strncpy(Title, title, MaxText);
  Title[MaxText-1] = '\0';
}


int ScreenMenu::addAction(const char *text, Action action, int id) {
  if (id < 0)
    id = NActions;
  strncpy(Texts[NActions], text, MaxText);
  Texts[NActions][MaxText-1] = '\0';
  Actions[NActions] = action;
  Checked[NActions] = -1;
  RadioButton[NActions] = false;
  Menus[NActions] = 0;
  IDs[NActions] = id;
  NActions++;
  return id;
}


int ScreenMenu::add(const char *text, int id) {
  return addAction(text, (Action)0, id);
}


int ScreenMenu::addCheckable(const char *text, bool checked, int id) {
  if (id < 0)
    id = NActions;
  strncpy(Texts[NActions], text, MaxText);
  Texts[NActions][MaxText-1] = '\0';
  Actions[NActions] = 0;
  Checked[NActions] = checked ? 1 : 0;
  RadioButton[NActions] = false;
  Menus[NActions] = 0;
  IDs[NActions] = id;
  NActions++;
  return id;
}


int ScreenMenu::addRadioButton(const char *text, bool checked, int id) {
  if (id < 0)
    id = NActions;
  strncpy(Texts[NActions], text, MaxText);
  Texts[NActions][MaxText-1] = '\0';
  Actions[NActions] = 0;
  Checked[NActions] = checked ? 1 : 0;
  RadioButton[NActions] = true;
  Menus[NActions] = 0;
  IDs[NActions] = id;
  if (checked) {
    for (int k=0; k<NActions; k++) {
      if (RadioButton[Index] && Checked[Index] > 0)
	Checked[k] = 0;
    }
  }
  NActions++;
  return id;
}


void ScreenMenu::addMenu(const char *text, ScreenMenu &menu) {
  strncpy(Texts[NActions], text, MaxText);
  Texts[NActions][MaxText-1] = '\0';
  Actions[NActions] = 0;
  Checked[NActions] = -1;
  RadioButton[NActions] = false;
  Menus[NActions] = &menu;
  IDs[NActions] = -1;
  NActions++;
}


bool ScreenMenu::checked(int id) const {
  for (int k=0; k<NActions; k++) {
    if (IDs[k] == id)
      return (Checked[k] > 0);
  }
  return false;
}


int ScreenMenu::checked() const {
  for (int k=0; k<NActions; k++) {
    if (RadioButton[k] && Checked[k] > 0)
      return IDs[k];
  }
  return -1;
}


void ScreenMenu::drawAction(int index, bool active) {
  uint16_t xoffs = Screen->defaultFont()->yAdvance;
  Canvas->fillScreen(0x0000);
  Canvas->setCursor(xoffs, Baseline);
  if (Checked[index] > 0) {
    if (RadioButton[index])
      Canvas->print("(X) ");
    else
      Canvas->print("[X] ");
  }
  else if (Checked[index] == 0) {
    if (RadioButton[index])
      Canvas->print("(");
    else
      Canvas->print("[");
    Canvas->setTextColor(0x0000);
    Canvas->print("X");
    Canvas->setTextColor(0xffff);
    if (RadioButton[index])
      Canvas->print(") ");
    else
      Canvas->print("] ");
  }
  Canvas->print(Texts[index]);
  if (active)
    Screen->screen()->drawBitmap(0, YPos[index], Canvas->getBuffer(),
				 Screen->width(), ActionHeight, BLACK, WHITE);
  else
    Screen->screen()->drawBitmap(0, YPos[index], Canvas->getBuffer(),
				 Screen->width(), ActionHeight, WHITE, BLACK);
}


void ScreenMenu::draw() {
  int n = NActions;
  if (strlen(Title) > 0)
    n++;
  Screen->clear();
  uint16_t height = Screen->height();
  uint16_t width = Screen->width();
  ActionHeight = height/n;
  uint16_t font_height = Screen->defaultFont()->yAdvance;
  Baseline = (ActionHeight - font_height)/2 + 4*font_height/5;
  if (Canvas == 0)
    Canvas = new GFXcanvas1(width, ActionHeight);
  Canvas->setRotation(0);
  Canvas->setFont(Screen->defaultFont());
  Canvas->setTextColor(0xffff);
  Canvas->setTextSize(1);
  Canvas->setTextWrap(false);
  uint16_t ypos = 0;
  if (strlen(Title) > 0) {
    Canvas->fillScreen(0x0000);
    Canvas->setCursor(0, Baseline);
    Canvas->print(Title);
    Screen->screen()->drawBitmap(0, ypos, Canvas->getBuffer(),
				 Screen->width(), ActionHeight, WHITE, BLACK);
    ypos += ActionHeight;
  }
  for (int k=0; k<NActions; k++) {
    YPos[k] = ypos;
    drawAction(k, k == Index);
    ypos += ActionHeight;
  }
  Screen->setBacklightOn();
}


void ScreenMenu::setCheckedAction(Action action) {
  CheckedAction = action;
}


void ScreenMenu::setCheckedReturns(bool returns) {
  CheckedReturns = returns;
}


int ScreenMenu::exec() {
  int index = -1;
  bool enabled = Buttons->enabled();
  Buttons->disable();
  draw();
  while (1) {
    Buttons->update();
    if (UpID >= 0 && Buttons->pressed(UpID)) {
      if (Index > 0) {
	drawAction(Index, false);
	Index--;
	drawAction(Index, true);
      }
    }
    else if (DownID >= 0 && Buttons->pressed(DownID)) {
      if (Index < NActions-1) {
	drawAction(Index, false);
	Index++;
	drawAction(Index, true);
      }
    }
    else if (SelectID >= 0 && Buttons->pressed(SelectID)) {
      drawAction(Index, false);
      delay(200);
      drawAction(Index, true);
      delay(200);
      Buttons->waitReleased(SelectID);
      index = IDs[Index];
      if (Actions[Index] != 0) {
	if (enabled)
	  Buttons->enable();
	Actions[Index](index);
	Buttons->disable();
	draw();
      }
      else if (Menus[Index] != 0) {
	Menus[Index]->exec();
	draw();
      }
      else if (Checked[Index] >= 0) {
	if (RadioButton[Index]) {
	  if (Checked[Index] == 0) {
	    Checked[Index] = 1;
	    if (Checked[Index] && CheckedAction != 0)
	      CheckedAction(IDs[Index]);
	    drawAction(Index, true);
	    for (int k=0; k<NActions; k++) {
	      if (k != Index && RadioButton[k] && Checked[k] > 0) {
		Checked[k] = 0;
		drawAction(k, false);
		break;
	      }
	    }
	    if (Checked[Index] && CheckedReturns) {
	      index = -1;
	      break;
	    }
	  }
	  else if (Checked[Index] && CheckedReturns) {
	    index = -1;
	    break;
	  }
	}
	else {
	  Checked[Index] = Checked[Index] > 0 ? 0 : 1;
	  if (Checked[Index] && CheckedAction != 0)
	    CheckedAction(IDs[Index]);
	  drawAction(Index, true);
	  if (Checked[Index] && CheckedReturns) {
	    index = -1;
	    break;
	  }
	}
      }
      else
	break;
    }
    else if (BackID >= 0 && Buttons->pressed(BackID)) {
      Buttons->waitReleased(BackID);
      index = -1;
      break;
    }
    yield();
  }
  if (enabled)
    Buttons->enable();
  return index;
}

