// Filename: event-text-input.js

const id = "EVENT_TEXT_INPUT";
const groups = ["EVENT_GROUP_DIALOGUE"];
const name = "Text Input";

const MAX_KEYBOARDS = 4;

const wrap8Bit = (val) => (256 + (val % 256)) % 256;

const decOct = (dec) => wrap8Bit(dec).toString(8).padStart(3, "0");

const fields = [].concat(
  [
    {
      key: "variable",
      label: "Select a Variable",
      type: "variable",
      defaultValue: "LAST_VARIABLE",
    },
    {
      key: "text_max_length",
      label: "Max Input Length",
      type: "variable",
      defaultValue: 8,
      min: 1,
    },
    {
      key: "text_position_x",
      label: "Text Position X (tiles)",
      type: "number",
      width: "50%",
      defaultValue: 6,
      min: 0,
      max: 20,
    },
    {
      key: "text_position_y",
      label: "Text Position Y (tiles)",
      type: "number",
      width: "50%",
      defaultValue: 3,
      min: 0,
      max: 18,
    },
    {
      key: "__scriptTabs",
      type: "tabs",
      defaultValue: "items",
      values: {
        items: "Keyboards",
        text: "Other",
      },
    },
  ],

  [
    {
      key: `showBorder`,
      label: "Show Border",
      type: "checkbox",
      defaultValue: true,
      width: "50%",
      conditions: [
        {
          key: "__scriptTabs",
          in: ["text"],
        },
      ],
    },
    {
      key: `clearPrevious`,
      label: "Clear Previous Content",
      type: "checkbox",
      defaultValue: true,
      width: "50%",
      conditions: [
        {
          key: "__scriptTabs",
          in: ["text"],
        },
      ],
    },
    {
      key: `showActors`,
      label: "Show Actors",
      type: "checkbox",
      defaultValue: false,
      width: "50%",
      conditions: [
        {
          key: "__scriptTabs",
          in: ["text"],
        },
      ],
    },
  ],

  [
    {
      key: "items",
      label: "Number of keyboards",
      type: "number",
      min: 1,
      max: MAX_KEYBOARDS,
      defaultValue: 2,
      conditions: [
        {
          key: "__scriptTabs",
          in: ["items"],
        },
      ],
    },
    {
      type: "break",
      conditions: [
        {
          key: "__scriptTabs",
          in: ["items"],
        },
      ],
    },
    ...Array(MAX_KEYBOARDS)
      .fill()
      .reduce((arr, _, i) => {
        const idx = i + 1;
        arr.push(
          {
            type: "break",
            conditions: [
              {
                key: "items",
                gte: idx,
              },
            ],
          },
          {
            key: `__collapseCase$${idx}`,
            label: `Keyboard ${idx}`,
            conditions: [
              {
                key: "items",
                gte: idx,
              },
              {
                key: "__scriptTabs",
                in: ["items"],
              },
            ],
            type: "collapsable",
            defaultValue: false,
          },
          {
            key: `keyboard_${idx}_layout`,
            type: "textarea",
            label: `Keyboard '${idx}' Layout`,
            placeholder: `Keyboard Layout ${idx}`,
            defaultValue: `1 2 3 4 5 6 7 8 9 0
Q W E R T Y U I O P
A S D F G H J K L :
Z X C V B N M < > ?
UPPER SPACE DONE`,
            flexBasis: "100%",
            conditions: [
              {
                key: "items",
                gte: idx,
              },
              {
                key: "__scriptTabs",
                in: ["items"],
              },
              {
                key: `__collapseCase$${idx}`,
                ne: true,
              },
            ],
          },
          {
            key: `keyboard${idx}_x`,
            label: "Keyboard X Position (tiles)",
            type: "number",
            min: 0,
            max: 20,
            width: "50%",
            defaultValue: 0,
            conditions: [
              {
                key: "items",
                gte: idx,
              },
              {
                key: "__scriptTabs",
                in: ["items"],
              },
              {
                key: `__collapseCase$${idx}`,
                ne: true,
              },
            ],
          },
          {
            key: `keyboard${idx}_y`,
            label: "Keyboard Y Position (tiles)",
            type: "number",
            min: 0,
            max: 18,
            width: "50%",
            defaultValue: 13,
            conditions: [
              {
                key: "items",
                gte: idx,
              },
              {
                key: "__scriptTabs",
                in: ["items"],
              },
              {
                key: `__collapseCase$${idx}`,
                ne: true,
              },
            ],
          }
        );
        return arr;
      }, []),
  ]
);

const compile = (input, helpers) => {
  const {
    appendRaw,
    _addComment,
    _overlayMoveTo,
    _loadStructuredText,
    _overlayClear,
    _overlayWait,
    _choice,
    _menuItem,
    _displayText,
    getVariableAlias,
    _addNL,
    variables,
    _setConst,
  } = helpers;

  const instantTextSpeedCode = `\\001\\1`;
  const variableAlias = getVariableAlias(input.variable);

  // Initialize the text string
  let textString = instantTextSpeedCode;

  // Variables to track overlay dimensions
  let minX = 20; // Max screen width in tiles
  let minY = 18; // Max screen height in tiles
  let maxX = 0;
  let maxY = 0;

  // Loop through each keyboard layout,
  // breaking each layout up by spaces and newlines
  const choices = [];
  for (let i = 0; i < input.items; i++) {
    const idx = i + 1;
    const itemText = input[`keyboard_${idx}_layout`];
    const fieldX = input[`keyboard${idx}_x`] || 0;
    const fieldY = input[`keyboard${idx}_y`] || 0;

    const x = decOct(1 + fieldX);
    const y = decOct(1 + fieldY);

    if (itemText) {
      // Calculate dimensions based on the keyboard layout
      const lines = itemText.trim().split("\n");
      const menuHeight = lines.length;
      const menuWidth = Math.max(...lines.map((line) => line.length));

      // Update overlay bounds
      minX = Math.min(minX, fieldX);
      minY = Math.min(minY, fieldY);
      maxX = Math.max(maxX, fieldX + menuWidth);
      maxY = Math.max(maxY, fieldY + menuHeight);

      // Add to text string
      textString += `\\003\\${x}\\${y}${itemText}`;

      // Split the keyboard layout into individual items
      lines.forEach((line) => {
        const items = line.split(" ");
        items.forEach((item) => {
          if (item) {
            choices.push(item);
          }
        });
      });
    }
  }

  // Add each item as a selection choice
  choices.forEach((choice, index) => {
    const choiceX = decOct(1 + (index % 10));
    const choiceY = decOct(1 + Math.floor(index / 10));
    textString += `\\003\\${choiceX}\\${choiceY}${choice}`;
  });

  const showActors = input.showActors;

  const speedIn = `.OVERLAY_SPEED_INSTANT`;
  const speedOut = `.OVERLAY_SPEED_INSTANT`;

  _addComment("Text Input Menu");

  // Clear previous content if needed
  if (input.clearPrevious) {
    appendRaw(`
      ; Copy the background tiles to the overlay
      VM_PUSH_CONST 0
      VM_PUSH_CONST 0
      VM_GET_INT16  .ARG1, _scroll_x
      VM_GET_INT16  .ARG0, _scroll_y      

      VM_RPN
        .R_INT8 0
        .R_INT8 0
        .R_INT8 20
        .R_INT8 18

        .R_INT8 0
        .R_REF  .ARG1
        .R_INT16 8
        .R_OPERATOR  .DIV
        .R_OPERATOR .MAX

        .R_INT8 0
        .R_REF  .ARG0
        .R_INT16 8
        .R_OPERATOR  .DIV
        .R_OPERATOR .MAX

        .R_STOP

      VM_OVERLAY_SET_SUBMAP_EX  .ARG5

      VM_POP  8
    `);
  }

  // Show actors on overlay if needed
  appendRaw(
    `VM_SET_CONST_UINT8 _show_actors_on_overlay, ${input.showActors ? 1 : 0}`
  );

  // Move overlay to starting position
  _overlayMoveTo(0, 0, ".OVERLAY_SPEED_INSTANT");

  // Load the structured text
  _loadStructuredText(textString);

  // Calculate overlay dimensions based on keyboard positions
  const overlayX = minX;
  const overlayY = minY;
  const overlayWidth = maxX - minX;
  const overlayHeight = maxY - minY;

  // Clear the overlay area where the keyboard will be displayed
  _overlayClear(
    overlayX,
    overlayY,
    overlayWidth,
    overlayHeight,
    ".UI_COLOR_WHITE",
    input.showBorder
  );

  // Display the text
  _displayText();

  // Wait for overlay to finish displaying
  _overlayWait(true, [".UI_WAIT_WINDOW", ".UI_WAIT_TEXT"]);

  // Wait for the user to make a choice
  _choice(variableAlias, [".UI_MENU_CANCEL_B"], input.items);

  // Close the overlay
  _overlayMoveTo(0, 0, speedOut);
  _overlayWait(true, [".UI_WAIT_WINDOW", ".UI_WAIT_TEXT"]);

  // Move overlay off-screen
  _overlayMoveTo(0, 18, ".OVERLAY_SPEED_INSTANT");

  // Disable actors on overlay
  appendRaw(`VM_SET_CONST_UINT8 _show_actors_on_overlay, 0`);
  _addNL();
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
