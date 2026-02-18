// commands.js - только определения блоков

// ----------------------------
//   COLORS
// ----------------------------
const COL_SCRIPT = 10;
const COL_TEXT   = 40;
const COL_GRAPH  = 140;
const COL_LOGIC  = 210;
const COL_ANIM   = 280;

// ----------------------------
//  SCRIPT ROOT BLOCK
// ----------------------------
Blockly.Blocks['SCRIPT'] = {
  init: function () {
    // Добавляем поле для ID скрипта
    this.appendDummyInput()
      .appendField("Script ID:")
      .appendField(new Blockly.FieldTextInput("main"), "SCRIPT_ID");
    
    this.appendDummyInput().appendField("Script Start");
    
    this.appendStatementInput("DO")
      .setCheck(null)
      .appendField("commands");
    
    this.setColour(COL_SCRIPT);
    this.setTooltip("Начало скрипта. Укажите ID скрипта (например: main, chapter1, etc)");
    this.setDeletable(false);
  }
};

Blockly.Blocks['FUNC'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("FUNC")
        .appendField(new Blockly.FieldTextInput("myfunc"), "NAME");
    this.appendStatementInput("BODY")
        .setCheck(null)
        .appendField("DO");
    this.setColour(200);
    this.setTooltip("Define a function");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['CALL'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("CALL")
        .appendField(new Blockly.FieldTextInput("myfunc"), "NAME");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(150);
    this.setTooltip("Call a function");
    this.setHelpUrl("");
  }
};

// ----------------------------
//  COMMANDS BLOCKS
// ----------------------------

// TXT
Blockly.Blocks['TXT'] = {
  init: function () {
    this.appendValueInput("TEXT")
      .setCheck("String")
      .appendField("TXT");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_TEXT);
  }
};

// CLTB
Blockly.Blocks['CLTB'] = {
  init: function () {
    this.appendDummyInput().appendField("CLTB");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_TEXT);
  }
};

// ROW
Blockly.Blocks['ROW'] = {
  init: function () {

    this.appendDummyInput().appendField("выполнить вместе");
    
    // Добавляем statement input для команд внутри ROW
    this.appendStatementInput("COMMANDS")
      .setCheck(null);
    
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_TEXT);
    this.setTooltip("Следующие команды выполняются сразу");
  }
};

// BG
Blockly.Blocks['BG'] = {
  init: function () {
    this.appendValueInput("IMG")
      .setCheck("String")
      .appendField("BG");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_GRAPH);
  }
};

// LD
Blockly.Blocks['LD'] = {
  init: function () {
    this.appendValueInput("CHAR")
      .setCheck("String")
      .appendField("LD");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_GRAPH);
  }
};

// LID
Blockly.Blocks['LID'] = {
  init: function () {
    this.appendValueInput("ID")
      .setCheck("Number")
      .appendField("LID");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_GRAPH);
  }
};

// CHSPR
Blockly.Blocks['CHSPR'] = {
  init: function () {
    this.appendValueInput("CHAR").setCheck("String").appendField("CHSPR");
    this.appendValueInput("SPRITE").setCheck("String").appendField("→");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_GRAPH);
  }
};

// ALIAS
Blockly.Blocks['ALIAS'] = {
  init: function () {
    this.appendValueInput("A").setCheck("String").appendField("ALIAS");
    this.appendValueInput("T").appendField("→");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_LOGIC);
  }
};

// SET
Blockly.Blocks['SET'] = {
  init: function () {
    this.appendValueInput("VAR").setCheck("String").appendField("SET");
    this.appendValueInput("VAL").setCheck(["String","Number"]);
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_LOGIC);
  }
};

Blockly.Blocks['variables_get'] = {
  init: function() {
    this.appendDummyInput()
      .appendField(new Blockly.FieldVariable("VAR_NAME"), "FIELD_NAME");
    this.setOutput(true, null);
    this.setColour("#5CA699");

  }
};

// Block for variable setter.
Blockly.Blocks['variables_set'] = {
  init: function() {
    // Поле для ввода названия переменной как строки
    this.appendValueInput("VAR_NAME")
        .setCheck("String")
        .appendField("set");
    
    this.appendValueInput("VALUE")
        .setCheck(null)
        .appendField("to");
      
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour("#5CA699");
  }
};


// RET
Blockly.Blocks['RET'] = {
  init: function () {
    this.appendValueInput("TO").setCheck("String").appendField("RET");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_LOGIC);
  }
};

// MV
Blockly.Blocks['MV'] = {
  init: function () {
    this.appendValueInput("CHAR").appendField("MV");
    this.appendValueInput("X").appendField("x");
    this.appendValueInput("Y").appendField("y");
    this.appendValueInput("t").appendField("t");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_ANIM);
  }
};

Blockly.Blocks['LDSIZE'] = {
  init: function () {
    this.appendValueInput("CHAR").appendField("LD with size");
    this.appendValueInput("W").appendField("w");
    this.appendValueInput("H").appendField("h");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_ANIM);
  }
};
Blockly.Blocks['LDXYWH'] = {
  init: function () {
    this.appendValueInput("CHAR").appendField("LD with x y width height");
    this.appendValueInput("X").appendField("x");
    this.appendValueInput("Y").appendField("y");
    this.appendValueInput("W").appendField("w");
    this.appendValueInput("H").appendField("h");
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(COL_ANIM);
  }
};


// ----------------------------
// TOOLBOX (CATEGORIES)
// ----------------------------
const COMMANDS_TOOLBOX = {
  "kind": "categoryToolbox",
  "contents": [
    { 
      "kind": "category", 
      "name": "Script", 
      "colour": COL_SCRIPT, 
      "contents":[ 
        { "kind":"block", "type":"SCRIPT" },
        { "kind":"block", "type":"FUNC" },
        { "kind":"block", "type":"CALL" } 
      ] 
    },
    { 
      "kind": "category", 
      "name": "Text", 
      "colour": COL_TEXT, 
      "contents":[
        { "kind":"block","type":"TXT" },
        { "kind":"block","type":"CLTB" },
        { "kind":"block","type":"ROW" },
        { "kind":"sep" },
        { "kind":"block","type":"text" }
      ]
    },
    { 
      "kind": "category", 
      "name": "Graphics", 
      "colour": COL_GRAPH, 
      "contents":[
        { "kind":"block","type":"BG" },
        { "kind":"block","type":"LD" },
        { "kind":"block","type":"LID" },
        { "kind":"block","type":"LDSIZE" },
        { "kind":"block","type":"LDXYWH" },
        { "kind":"block","type":"CHSPR" }
      ]
    },
    { 
      "kind": "category", 
      "name": "Logic", 
      "colour": COL_LOGIC, 
      "contents":[
        { "kind":"block","type":"ALIAS" },
        { "kind":"block","type":"SET" },
        { "kind":"block","type":"RET" }
      ]
    },
    { 
      "kind": "category", 
      "name": "Animation", 
      "colour": COL_ANIM, 
      "contents":[
        { "kind":"block","type":"MV" }
      ]
    },
    { 
      "kind": "category", 
      "name": "Values", 
      "colour": "#5CA699", 
      "contents":[
        { "kind":"block","type":"text" },
        { "kind": "block", "type":"variables_get"},
        { "kind": "block", "type":"variables_set"},
        { "kind":"block","type":"math_number" }
      ]
    }
  ]
};

console.log('Блоки команд определены');

window.COMMANDS_TOOLBOX = COMMANDS_TOOLBOX;