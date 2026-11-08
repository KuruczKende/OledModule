# HOWTO: add a new screen

This project keeps a small screen stack and a shared UI drawing layer. The most important rule is:

- every screen must be initialized once in `Screens.c`
- every screen ID must be declared in `Screens.h`
- the screen must be assigned a button handler if it wants user input
- the screen must initialize its own UI elements before it is opened

This document explains the usual workflow and the common pitfalls.

## 1. Add a screen ID

Open `Screens.h` and extend the `E_SCREENS` enum.

```c
typedef enum{
  eSCREEN_TEST,
  eSCREEN_IRON,
  eSCREEN_SETTINGS,
  eSCREEN_NUM
}E_SCREENS;
```

Important:

- `eSCREEN_NUM` must remain the last value
- do not insert new IDs after `eSCREEN_NUM`
- all screen IDs must be contiguous and zero-based

## 2. Add the screen initialization function

Create a function in the screen source file, typically next to the other screen definitions.

Example pattern:

```c
  /* initialize widgets / drawables here */

void settingsScreenInit(C_SCREEN* pcScreen, void* ptr){

  *pcScreen = (C_SCREEN){
    .cUI = (C_UI_SCREEN){
      .pcElements = cSettingsScreenElements,
      .u8ElementNum = sizeof(cSettingsScreenElements) / sizeof(cSettingsScreenElements[0]),
    },
    .bofpButtonEvent = settingsButtonEvent,
    .u8InnerState = 0,
  };
}
```

Rules to watch for:

- `pcScreen->cUI.pcElements` must point to an array of `C_UI_DRAWABLE`
- `u8ElementNum` must be computed with `sizeof(...) / sizeof(...)`
- if the screen has no button handling, set `bofpButtonEvent = NULL`
- `u8InnerState` is a per-screen state variable; initialize it explicitly

## 3. Add button event handling (if needed)

If the screen reacts to buttons, define a callback with the same signature as other screens:

```c
bool settingsButtonEvent(C_SCREEN* pcScreen, C_BUTTONS_EVENT* pcEvent){
  if (pcEvent->eEvent == eBUTTON_CLICKED && pcEvent->u8Button == eBUTTON_UP) {
    pcScreen->u8InnerState++;
  }
  return true;
}
```

Common pitfalls:

- `pcEvent->u8Button` values come from `eBUTTON_*` in `Screens.h`
- using the wrong event type leads to undefined behavior
- the callback must return a `bool` and should normally return `true` when handled (i don't do anything with the return yet)
- screen-local state should be read/written via `pcScreen->u8InnerState` or other screen members

## 4. Register the screen in `Screens.c`

Open `Screens.c` and initialize it inside `vSCREEN_Init()`.

```c
void vSCREEN_Init(){
  boIOE_Init();
  u8ScreenBufferSize = 0;

  testScreenInit(&acScreen[eSCREEN_TEST], NULL);
  ironScreenInit(&acScreen[eSCREEN_IRON], NULL);
  settingsScreenInit(&acScreen[eSCREEN_SETTINGS], NULL);

  /* button config continues here */
}
```

This is the step that actually makes the screen available to the system. If you forget it, the screen ID exists but is never usable.

## 5. Make sure the screen is compatible with the UI model

Each screen uses `C_UI_DRAWABLE` elements. A drawable can be:

- `eUI_DRAWABLE_TYPE_TEXT`
- `eUI_DRAWABLE_TYPE_SHAPE`
- `eUI_DRAWABLE_TYPE_WINDOW`

Example:

```c
C_UI_DRAWABLE cSettingsScreenElements[] = {
  {
    .eType = eUI_DRAWABLE_TYPE_TEXT,
    .i16PosX = 5,
    .i16PosY = 5,
    .boInversed = false,
    .pu8ConditionRef = NULL,
    .u8ConditionRefValue = 0,
    .cText = {
      .eType = eUI_TEXT_TYPE_VALUE_STR,
      .pchGet = SettingsTitleText,
      .pvCtx = NULL,
      .Font = &Font_7x10
    }
  }
};
```

Things to check:

- `Font` must point to a valid font object
- `pchGet`, `u8Get`, `i8Get`, etc. must match the selected `eUI_TEXT_TYPE`
- `u8ElementNum` must match the actual array size
- the coordinates should stay in the visible OLED range

## 6. Watch for initialization traps

These are the most common mistakes when adding a new screen:

### a) forgetting to initialize the screen instance

If you add a new enum value but never call its init function, the screen cache will contain uninitialized data.

### b) invalid `C_SCREEN` assignment

Each screen struct must contain:

- a valid `C_UI_SCREEN` object
- a button callback pointer or `NULL`
- a valid `u8InnerState` value

### c) wrong button wiring

The button IDs in `Screens.h` must match the physical buttons configured in `vSCREEN_Init()`.

Example:

```c
cButton.ePin = eIOE_PIN_P5;
boAddButton(eBUTTON_UP, &cButton);
```

If the screen expects `eBUTTON_UP` but the hardware is mapped to another pin, the input will appear to be broken.

### d) mismatched text getter types

For example, if you set:

```c
.eType = eUI_TEXT_TYPE_VALUE_U8,
```

then the corresponding getter must be a function returning `uint8_t`:

```c
uint8_t MyValue(void *pvCtx) { return 42; }
```

Using a wrong getter signature leads to undefined behavior or bad rendering.

### e) wrong element count

This is a silent bug. `u8ElementNum` must exactly match the number of entries in the drawable array.

## 7. Open the screen from elsewhere

Once the screen is registered and initialized, you can open it with:

```c
boSCREEN_Open(eSCREEN_SETTINGS);
```

Or replace the current screen with:

```c
boSCREEN_Switch(eSCREEN_SETTINGS);
```

`boSCREEN_Open()` pushes a new screen on the stack, while `boSCREEN_Switch()` replaces the currently visible screen without adding a new layer.

## 8. Recommended workflow

Use this order every time you add a new screen:

1. Add the enum value in `Screens.h`
2. Create the screen-specific elements and callback functions
3. Add the init function for the screen
4. Register it in `vSCREEN_Init()` in `Screens.c`
5. Map any input buttons to the right `eBUTTON_*` values
6. Compile and run the UI to verify the screen draws and responds correctly

## 9. Minimal example

```c
static char achTitle[] = "Title";

char* SettingsTitleText(void* pvCtx){
  return achTitle;
}

C_UI_DRAWABLE cSettingsScreenElements[] = {
  {
    .eType = eUI_DRAWABLE_TYPE_TEXT,
    .i16PosX = 10,
    .i16PosY = 10,
    .boInversed = false,
    .pu8ConditionRef = NULL,
    .u8ConditionRefValue = 0,
    .cText = {
      .eType = eUI_TEXT_TYPE_VALUE_STR,
      .pchGet = SettingsTitleText,
      .pvCtx = NULL,
      .Font = &Font_7x10
    }
  }
};

bool settingsButtonEvent(C_SCREEN* pcScreen, C_BUTTONS_EVENT* pcEvent){
  if (pcEvent->eEvent == eBUTTON_CLICKED) {
    switch (pcEvent->u8Button) {
    case eBUTTON_UP:
      pcScreen->u8InnerState--;
      if (pcScreen->u8InnerState >= 3) {
        pcScreen->u8InnerState = 2;
      }
      break;
    case eBUTTON_DOWN:
      pcScreen->u8InnerState++;
      if (pcScreen->u8InnerState >= 3) {
        pcScreen->u8InnerState = 0;
      }
      break;
    case eBUTTON_MIDDLE:
      boSCREEN_Switch(eSCREEN_TEST);
      break;
    default:
      break;
    }
  }
  return true;
}

void settingsScreenInit(C_SCREEN* pcScreen, void* ptr){
  cSettingsScreenElements[0].pu8ConditionRef = &pcScreen->u8InnerState;

  *pcScreen = (C_SCREEN){
    .cUI = (C_UI_SCREEN){
      .pcElements = cSettingsScreenElements,
      .u8ElementNum = sizeof(cSettingsScreenElements) / sizeof(cSettingsScreenElements[0]),
    },
    .bofpButtonEvent = settingsButtonEvent,
    .u8InnerState = 0,
  };
}
```

If you follow this flow, new screens will fit the existing architecture without breaking the screen stack or input logic.
