const id = "EVENT_UNPACK_TILE_VARIABLE";
const groups = ["Tiles"];
const name = "Unpack Tile Variable";

const fields = [
  {
    key: `packed_tile_variable`,
    label: `Packed Tile Variable`,
    type: "variable",
    default: "LAST_VARIABLE",
  },
  {
    key: `unpack_type`,
    label: `Unpack Type`,
    type: "select",
    options: [
      ["x", "Tile X"],
      ["y", "Tile Y"],
    ],
    default: "x",
  },
  {
    key: `result_variable`,
    label: `Result Variable`,
    type: "variable",
    default: "LAST_VARIABLE",
  },
];

const compile = (input, helpers) => {
  const { _setToVariable } = helpers;

  const packedTileVariable = input.packed_tile_variable;
  const unpackType = input.unpack_type;
  const resultVariable = input.result_variable;

  let packedValue = packedTileVariable.value;
  let resultValue;

  if (unpackType === "x") {
    resultValue = `${packedValue} & 0x1F`;
  } else {
    resultValue = `(${packedValue} >> 5) & 0x1F`;
  }

	if (resultVariable.type === "variable") {
		_setToVariable(resultVariable, resultValue);
	}
	else {
		resultVariable.value = resultValue;
	}
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
};
