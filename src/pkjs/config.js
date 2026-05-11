module.exports = [
  {
    "type": "heading",
    "defaultValue": "Goofster Settings"
  },
  {
    "type": "text",
    "defaultValue": "Here you may customise your Goofster to your heart's content, as long as your heart is content with changing the background and nose colours."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0xAAFFAA",
        "label": "Background Color"
      },
      {
        "type": "color",
        "messageKey": "NoseColor",
        "defaultValue": "0xFF0000",
        "label": "Nose Color"
      },
      {
        "type": "heading",
        "defaultValue": "Settings"
      },
      {
        "type": "toggle",
        "messageKey": "NoseSeconds",
        "label": "Enable Nose Seconds",
        "defaultValue": false
      },

    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
