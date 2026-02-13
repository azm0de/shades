#ifndef SHADES_COMMANDS_H
#define SHADES_COMMANDS_H

//=============================================================================
// Menu Command IDs
//=============================================================================

// File menu (40xx)
#define ID_FILE_NEW         4001
#define ID_FILE_OPEN        4002
#define ID_FILE_SAVE        4003
#define ID_FILE_SAVE_AS     4004
#define ID_FILE_IMPORT      4005
#define ID_FILE_EXPORT      4006
#define ID_FILE_RECENT_1    4011
#define ID_FILE_RECENT_2    4012
#define ID_FILE_RECENT_3    4013
#define ID_FILE_EXIT        4099

// Edit menu (41xx)
#define ID_EDIT_UNDO        4101
#define ID_EDIT_REDO        4102
#define ID_EDIT_COPY        4103
#define ID_EDIT_PASTE       4104
#define ID_EDIT_RESET       4105

// View menu (42xx)
#define ID_VIEW_GALLERY       4201
#define ID_VIEW_COLOR_EDITOR  4202
#define ID_VIEW_PREVIEW       4203
#define ID_VIEW_ZOOM_IN       4210
#define ID_VIEW_ZOOM_OUT      4211
#define ID_VIEW_ZOOM_RESET    4212
#define ID_VIEW_REFRESH       4213

// Tools menu (43xx)
#define ID_TOOLS_VALIDATE     4301
#define ID_TOOLS_PERFORMANCE  4302
#define ID_TOOLS_APPLY        4303
#define ID_TOOLS_DISABLE      4304
#define ID_TOOLS_SETTINGS     4305

// Help menu (44xx)
#define ID_HELP_DOCUMENTATION 4401
#define ID_HELP_GITHUB        4402
#define ID_HELP_REPORT        4403
#define ID_HELP_UPDATES       4404
#define ID_HELP_ABOUT         4405

// Context menu (45xx)
#define ID_CONTEXT_APPLY      4501
#define ID_CONTEXT_EDIT       4502
#define ID_CONTEXT_DUPLICATE  4503
#define ID_CONTEXT_EXPORT     4504
#define ID_CONTEXT_SHARE      4505
#define ID_CONTEXT_DELETE     4506
#define ID_CONTEXT_PROPERTIES 4507

// Gallery buttons (46xx)
#define IDC_BTN_NEW           4601
#define IDC_BTN_IMPORT        4602
#define IDC_BTN_EXPORT        4603

// Status bar parts
#define STATUS_PART_THEME       0
#define STATUS_PART_STATUS      1
#define STATUS_PART_PERFORMANCE 2
#define STATUS_PART_VERSION     3
#define STATUS_PART_COUNT       4

#endif // SHADES_COMMANDS_H
