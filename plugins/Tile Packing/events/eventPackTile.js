const id = "EVENT_PACK_TILE_VARIABLE";
const groups = ["Tiles"];
const name = "Pack Tile Variable";

const fields = [
  {
    key: `packed_tile_variable`,
    label: `Packed Tile Variable`,
    type: "variable",
    default: "LAST_VARIABLE",
  },
  {
    key: `tile_x`,
    label: `Tile X`,
    type: "union",
    types: ["number", "variable"],
    defaultType: "number",
    min: 0,
    defaultValue: {
      number: 0,
      variable: "LAST_VARIABLE",
    },
    width: "50%",
  },
  {
    key: `tile_y`,
    label: `Tile Y`,
    type: "union",
    types: ["number", "variable"],
    defaultType: "number",
    min: 0,
    defaultValue: {
      number: 0,
      variable: "LAST_VARIABLE",
    },
    width: "50%",
  },
];

const compile = (input, helpers) => {
  const {
    variableFromUnion,
    _setToVariable
  } = helpers;

  const tileX = variableFromUnion(input.tile_x);
  const tileY = variableFromUnion(input.tile_y);
  const packedTileVariable = input.packed_tile_variable;

  // Pack the tile X and Y positions into a single variable
  const packedValue = `((${tileY} & 0x1F) << 5) | (${tileX} & 0x1F)`;

  // Set the packed value to the specified variable
  _setToVariable(packedTileVariable, packedValue);
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
};
