/**
спасибо claude ai
 */

(function () {
  "use strict";
 
  const STORAGE_KEY = "vnEditorFileList";
 
  /** @type {string[]} */
  let fileList = [];

  function loadFiles() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY);
      fileList = raw ? JSON.parse(raw) : [];
    } catch (e) {
      fileList = [];
    }
  }
 
  function saveFiles() {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(fileList));
  }
 
  function injectStyles() {
    const style = document.createElement("style");
    style.textContent = `
      #fm-toggle-btn {
        position: fixed;
        top: 12px;
        right: 16px;
        z-index: 9998;
        padding: 8px 14px;
        background: #2d6cdf;
        color: #fff;
        border: none;
        border-radius: 6px;
        font-size: 14px;
        cursor: pointer;
        box-shadow: 0 2px 6px rgba(0,0,0,.25);
      }
      #fm-toggle-btn:hover { background: #1f56b8; }
 
      #fm-overlay {
        display: none;
        position: fixed;
        inset: 0;
        background: rgba(0,0,0,.45);
        z-index: 9999;
        align-items: center;
        justify-content: center;
      }
      #fm-overlay.open { display: flex; }
 
      #fm-modal {
        background: #fff;
        width: 420px;
        max-width: 92vw;
        max-height: 80vh;
        border-radius: 10px;
        box-shadow: 0 10px 30px rgba(0,0,0,.3);
        display: flex;
        flex-direction: column;
        overflow: hidden;
        font-family: Arial, sans-serif;
      }
      #fm-modal header {
        padding: 14px 16px;
        background: #2d6cdf;
        color: #fff;
        display: flex;
        justify-content: space-between;
        align-items: center;
      }
      #fm-modal header h2 { margin: 0; font-size: 16px; }
      #fm-close-btn {
        background: transparent;
        border: none;
        color: #fff;
        font-size: 20px;
        cursor: pointer;
        line-height: 1;
      }
      #fm-body { padding: 14px 16px; overflow-y: auto; }
      #fm-add-row {
        display: flex;
        gap: 8px;
        margin-bottom: 12px;
      }
      #fm-add-row input {
        flex: 1;
        padding: 8px 10px;
        border: 1px solid #ccc;
        border-radius: 6px;
        font-size: 14px;
      }
      #fm-add-row button {
        padding: 8px 12px;
        background: #2d6cdf;
        color: #fff;
        border: none;
        border-radius: 6px;
        cursor: pointer;
        font-size: 14px;
      }
      #fm-add-row button:hover { background: #1f56b8; }
 
      #fm-list { list-style: none; margin: 0; padding: 0; }
      #fm-list li {
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 8px 10px;
        border: 1px solid #e2e2e2;
        border-radius: 6px;
        margin-bottom: 6px;
        background: #fafafa;
      }
      #fm-list li span {
        font-size: 13px;
        word-break: break-all;
        margin-right: 8px;
      }
      #fm-list li .fm-actions { display: flex; gap: 6px; flex-shrink: 0; }
      #fm-list li button {
        border: none;
        border-radius: 5px;
        padding: 5px 8px;
        font-size: 12px;
        cursor: pointer;
      }
      .fm-insert-btn { background: #2fa84f; color: #fff; }
      .fm-insert-btn:hover { background: #24893f; }
      .fm-remove-btn { background: #e05353; color: #fff; }
      .fm-remove-btn:hover { background: #c73f3f; }
 
      #fm-empty {
        color: #888;
        font-size: 13px;
        text-align: center;
        padding: 10px 0;
      }
    `;
    document.head.appendChild(style);
  }
 
  let listEl, inputEl, overlayEl;
 
  function buildUI() {
    const toggleBtn = document.createElement("button");
    toggleBtn.id = "fm-toggle-btn";
    toggleBtn.textContent = "📁 Файлы";
    toggleBtn.onclick = openModal;
    document.body.appendChild(toggleBtn);
 
    overlayEl = document.createElement("div");
    overlayEl.id = "fm-overlay";
    overlayEl.innerHTML = `
      <div id="fm-modal">
        <header>
          <h2>Список файлов</h2>
          <button id="fm-close-btn" title="Закрыть">&times;</button>
        </header>
        <div id="fm-body">
          <div id="fm-add-row">
            <input type="text" id="fm-input" placeholder="например: hero_smile.png" />
            <button id="fm-add-btn">Добавить</button>
          </div>
          <ul id="fm-list"></ul>
        </div>
      </div>
    `;
    document.body.appendChild(overlayEl);
 
    listEl = overlayEl.querySelector("#fm-list");
    inputEl = overlayEl.querySelector("#fm-input");
 
    overlayEl.querySelector("#fm-close-btn").onclick = closeModal;
    overlayEl.addEventListener("click", (e) => {
      if (e.target === overlayEl) closeModal();
    });
 
    overlayEl.querySelector("#fm-add-btn").onclick = addFileFromInput;
    inputEl.addEventListener("keydown", (e) => {
      if (e.key === "Enter") addFileFromInput();
    });
  }
 
  function openModal() {
    overlayEl.classList.add("open");
    inputEl.focus();
  }
  function closeModal() {
    overlayEl.classList.remove("open");
  }
 
  function addFileFromInput() {
    const name = inputEl.value.trim();
    if (!name) return;
    if (fileList.includes(name)) {
      inputEl.value = "";
      return;
    }
    fileList.push(name);
    saveFiles();
    inputEl.value = "";
    renderList();
    refreshDropdownBlocks();
  }
 
  function removeFile(name) {
    fileList = fileList.filter((f) => f !== name);
    saveFiles();
    renderList();
    refreshDropdownBlocks();
  }
 
  function renderList() {
    listEl.innerHTML = "";
    if (fileList.length === 0) {
      const empty = document.createElement("div");
      empty.id = "fm-empty";
      empty.textContent = "Пока нет ни одного файла";
      listEl.appendChild(empty);
      return;
    }
    fileList.forEach((name) => {
      const li = document.createElement("li");
 
      const span = document.createElement("span");
      span.textContent = name;
      li.appendChild(span);
 
      const actions = document.createElement("div");
      actions.className = "fm-actions";
 
      const insertBtn = document.createElement("button");
      insertBtn.className = "fm-insert-btn";
      insertBtn.textContent = "Вставить блок";
      insertBtn.onclick = () => insertBlockForFile(name);
      actions.appendChild(insertBtn);
 
      const removeBtn = document.createElement("button");
      removeBtn.className = "fm-remove-btn";
      removeBtn.textContent = "Удалить";
      removeBtn.onclick = () => removeFile(name);
      actions.appendChild(removeBtn);
 
      li.appendChild(actions);
      listEl.appendChild(li);
    });
  }
 
  let insertOffset = 0;
 
  function getDropdownOptions() {
    if (fileList.length === 0) {
      return [["(нет файлов)", "NONE"]];
    }
    return fileList.map((f) => [f, f]);
  }
 
  function registerBlocklyBlock() {
    if (typeof Blockly === "undefined") {
      console.warn("file-manager.js: Blockly не найден, подключите скрипт после Blockly.");
      return;
    }
 
    Blockly.Blocks["FILE_VALUE"] = {
      init: function () {
        this.appendDummyInput()
          .appendField("📁")
          .appendField(new Blockly.FieldDropdown(getDropdownOptions), "FILE");
        this.setOutput(true, null);
        this.setColour(200);
        this.setTooltip("Имя файла из списка файлов проекта");
      },
    };
 
    Blockly.JavaScript.forBlock["FILE_VALUE"] = function (block) {
      const value = block.getFieldValue("FILE");
      const text = value === "NONE" ? "" : value;
      return [`"${text}"`, Blockly.JavaScript.ORDER_ATOMIC];
    };
  }
 
  function refreshDropdownBlocks() {
    if (typeof workspace === "undefined" || !workspace) return;
    const blocks = workspace.getBlocksByType("FILE_VALUE", false);
    blocks.forEach((b) => {
      const field = b.getField("FILE");
      if (field && typeof field.forceRerender === "function") {
        field.forceRerender();
      }
    });
  }
 
  function insertBlockForFile(name) {
    if (typeof workspace === "undefined" || !workspace) {
      alert("Рабочая область Blockly (workspace) не найдена.");
      return;
    }
    const block = workspace.newBlock("FILE_VALUE");
    block.initSvg();
    block.render();

    const x = 40 + (insertOffset % 6) * 30;
    const y = 40 + (insertOffset % 6) * 30;
    insertOffset++;
    block.moveBy(x, y);
 
    if (block.getField("FILE")) {
      block.setFieldValue(name, "FILE");
    }
 
    closeModal();
  }
 
  function hookSaveLoad() {
    const saveBtn = document.getElementById("saveBtn");
    const loadInput = document.getElementById("loadInput");
 
    if (saveBtn && typeof workspace !== "undefined" && workspace) {
      saveBtn.onclick = () => {
        const data = {
          workspace: Blockly.serialization.workspaces.save(workspace),
          files: fileList,
        };
        const blob = new Blob([JSON.stringify(data, null, 2)], {
          type: "application/json",
        });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = "project.json";
        a.click();
        URL.revokeObjectURL(url);
      };
    }
 
    function applyLoadedJson(json) {
      const workspaceState = json.workspace || json;
      if (Array.isArray(json.files)) {
        fileList = json.files;
        saveFiles();
        renderList();
      }
      Blockly.serialization.workspaces.load(workspaceState, workspace);
      refreshDropdownBlocks();
    }
 
    if (loadInput) {
      loadInput.onchange = (e) => {
        const file = e.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = function () {
          const json = JSON.parse(reader.result);
          applyLoadedJson(json);
        };
        reader.readAsText(file);
      };
    }
 
    document.addEventListener("drop", (e) => {
      const file = e.dataTransfer.files && e.dataTransfer.files[0];
      if (!file || !file.name.endsWith(".json")) return;
      const reader = new FileReader();
      reader.onload = function () {
        try {
          const json = JSON.parse(reader.result);
          if (json && (json.files || json.workspace)) {
            applyLoadedJson(json);
          }
        } catch (err) {

        }
      };
      reader.readAsText(file);
    });
  }
 
  function init() {
    loadFiles();
    injectStyles();
    buildUI();
    renderList();
    registerBlocklyBlock();
    hookSaveLoad();
  }
 
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
 
