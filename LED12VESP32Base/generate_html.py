"""
py code to generate the web pages for configuration
"""

from generate_css import gen_css


LIGHT_MAX_BRIGHTNESS: int = 255
LIGHT_MIN_BRIGHTNESS: int = 1
LIGHT_DEF_BRIGHTNESS: int = 210

LIGHT_DEF_TRANSITION_TIME: int = 1000
LIGHT_MIN_TRANSITION_TIME: int = 1
LIGHT_MAX_TRANSITION_TIME: int = 10000

LIGHT_DEF_BRIGHTNESS_STEP: int = 8
LIGHT_MIN_BRIGHTNESS_STEP: int = 1
LIGHT_MAX_BRIGHTNESS_STEP: int = 255

NIGHT_MAX_BRIGHTNESS: int = 128
NIGHT_MIN_BRIGHTNESS: int = 1
NIGHT_DEF_BRIGHTNESS: int = 8

NIGHT_DEF_DURATION: int = 30
NIGHT_MIN_DURATION: int = 1
NIGHT_MAX_DURATION: int = 600

LDR_DEF_VALUE: int = 30
LDR_MIN_VALUE: int = 1
LDR_MAX_VALUE: int = 4095

PRS_DEF_DIST_VALUE: int = 30
PRS_MIN_DIST_VALUE: int = 30
PRS_MAX_DIST_VALUE: int = 800

PRS_MIN_NRG_VALUE: int = 0
PRS_MAX_NRG_VALUE: int = 100

KEY_TITLE = "title"
KEY_EXPLANATION = "explanation"
KEY_TYPE = "type"
KEY_LABEL = "label"
KEY_LABELFOR = "label_for"
KEY_DEFAULT = "default"
KEY_MIN = "min"
KEY_MAX = "max"
KEY_GROUPS = "groups"
KEY_DETAILS = "details"
KEY_ALLOW_EMPTY = "allow_empty"

VAL_TYPE_BOOL = "bool"
VAL_TYPE_STRING = "string"
VAL_TYPE_PASSWORD = "password"
VAL_TYPE_UINT8 = "uint8_t"
VAL_TYPE_UINT16 = "uint16_t"
VAL_TYPE_IPV4 = "ipv4"

VAL_TYPES = [
    VAL_TYPE_BOOL,
    VAL_TYPE_STRING,
    VAL_TYPE_PASSWORD,
    VAL_TYPE_UINT8,
    VAL_TYPE_UINT16,
    VAL_TYPE_IPV4
]

DEFINITION = {
    "light": {
        KEY_TITLE: "Light",
        KEY_GROUPS: [
            {
                KEY_EXPLANATION: f"Lower values mean lower brightness. Allowed values: {LIGHT_MIN_BRIGHTNESS}..{LIGHT_MAX_BRIGHTNESS}.",
                "OnBrightness": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Brighteness in light mode",
                    KEY_LABELFOR: "obr",
                    KEY_DEFAULT: LIGHT_DEF_BRIGHTNESS,
                    KEY_MIN: LIGHT_MIN_BRIGHTNESS,
                    KEY_MAX: LIGHT_MAX_BRIGHTNESS,
                },
                "MaxBrightness": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Max brighteness in light mode",
                    KEY_LABELFOR: "mbr",
                    KEY_DEFAULT: LIGHT_DEF_BRIGHTNESS,
                    KEY_MIN: LIGHT_MIN_BRIGHTNESS,
                    KEY_MAX: LIGHT_MAX_BRIGHTNESS,
                },
            },
        ],
    },
    "nightlight": {
        KEY_TITLE: "Nightlight",
        KEY_GROUPS: [
            {
                "AllowNightLight": {
                    KEY_TYPE: VAL_TYPE_BOOL,
                    KEY_LABEL: "Allow nightlight mode",
                    KEY_LABELFOR: "alnl",
                    KEY_DEFAULT: True,
                },
            },
            {
                KEY_EXPLANATION: f"Lower values mean lower brightness. Allowed values: {NIGHT_MIN_BRIGHTNESS}..{NIGHT_MAX_BRIGHTNESS}.",
                "NightLightBrightness": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Brighteness in nightlight mode",
                    KEY_LABELFOR: "mnlb",
                    KEY_DEFAULT: NIGHT_DEF_BRIGHTNESS,
                    KEY_MIN: NIGHT_MIN_BRIGHTNESS,
                    KEY_MAX: NIGHT_MAX_BRIGHTNESS,
                },
                "MaxNightLightBrightness": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Max brighteness in nightlight mode",
                    KEY_LABELFOR: "nlbr",
                    KEY_DEFAULT: NIGHT_MAX_BRIGHTNESS,
                    KEY_MIN: NIGHT_MIN_BRIGHTNESS,
                    KEY_MAX: NIGHT_MAX_BRIGHTNESS,
                },
            },
            {
                KEY_EXPLANATION: f"Allowed values: {NIGHT_MIN_DURATION}..{NIGHT_MAX_DURATION}.",
                "NightLightOnDuration": {
                    KEY_TYPE: VAL_TYPE_UINT16,
                    KEY_LABEL: "On duration (seconds)",
                    KEY_LABELFOR: "odu",
                    KEY_DEFAULT: NIGHT_DEF_DURATION,
                    KEY_MIN: NIGHT_MIN_DURATION,
                    KEY_MAX: NIGHT_MAX_DURATION,
                },
            },
            {
                KEY_EXPLANATION: f"Brightness detection, lower values mean lower brightness. Allowed values: {LDR_MIN_VALUE}..{LDR_MAX_VALUE}.",
                "NightLightThreshold": {
                    KEY_TYPE: VAL_TYPE_UINT16,
                    KEY_LABEL: "LDR Threshold",
                    KEY_LABELFOR: "nllt",
                    KEY_DEFAULT: LDR_DEF_VALUE,
                    KEY_MIN: LDR_MIN_VALUE,
                    KEY_MAX: LDR_MAX_VALUE,
                },
            },
        ]
    },
    "presence": {
        KEY_TITLE: "Presence detection",
        KEY_GROUPS: [
            {
                KEY_TITLE: "Distance",
                KEY_EXPLANATION: f"Distance is a generic value (not meters or the like). Allowed values: {PRS_MIN_DIST_VALUE}..{PRS_MAX_DIST_VALUE}.",
                "MaxMovingTargetDistance": {
                    KEY_TYPE: VAL_TYPE_UINT16,
                    KEY_LABEL: "Max moving target distance",
                    KEY_LABELFOR: "mamd",
                    KEY_DEFAULT: PRS_MAX_DIST_VALUE,
                    KEY_MIN: PRS_MIN_DIST_VALUE,
                    KEY_MAX: PRS_MAX_DIST_VALUE,
                },
                "MinMovingTargetDistance": {
                    KEY_TYPE: VAL_TYPE_UINT16,
                    KEY_LABEL: "Min moving target distance",
                    KEY_LABELFOR: "mimd",
                    KEY_DEFAULT: PRS_MIN_DIST_VALUE,
                    KEY_MIN: PRS_MIN_DIST_VALUE,
                    KEY_MAX: PRS_MAX_DIST_VALUE,
                },
                "MaxStationaryTargetDistance": {
                    KEY_TYPE: VAL_TYPE_UINT16,
                    KEY_LABEL: "Max stationary target distance",
                    KEY_LABELFOR: "masd",
                    KEY_DEFAULT: PRS_MAX_DIST_VALUE,
                    KEY_MIN: PRS_MIN_DIST_VALUE,
                    KEY_MAX: PRS_MAX_DIST_VALUE,
                },
                "MinStationaryTargetDistance": {
                    KEY_TYPE: VAL_TYPE_UINT16,
                    KEY_LABEL: "Min stationary target distance",
                    KEY_LABELFOR: "misd",
                    KEY_DEFAULT: PRS_MIN_DIST_VALUE,
                    KEY_MIN: PRS_MIN_DIST_VALUE,
                    KEY_MAX: PRS_MAX_DIST_VALUE,
                },
            },
            {
                KEY_TITLE: "Energy",
                KEY_EXPLANATION: f'Read "energy" as "certainty". Allowed values: {PRS_MIN_NRG_VALUE}..{PRS_MAX_NRG_VALUE}.',
                "MaxMovingTargetEnergy": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Max moving target energy",
                    KEY_LABELFOR: "mame",
                    KEY_DEFAULT: PRS_MAX_NRG_VALUE,
                    KEY_MIN: PRS_MIN_NRG_VALUE,
                    KEY_MAX: PRS_MAX_NRG_VALUE,
                },
                "MinMovingTargetEnergy": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Min moving target energy",
                    KEY_LABELFOR: "mime",
                    KEY_DEFAULT: PRS_MIN_NRG_VALUE,
                    KEY_MIN: PRS_MIN_NRG_VALUE,
                    KEY_MAX: PRS_MAX_NRG_VALUE,
                },
                "MaxStationaryTargetEnergy": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Max stationary target energy",
                    KEY_LABELFOR: "mase",
                    KEY_DEFAULT: PRS_MAX_NRG_VALUE,
                    KEY_MIN: PRS_MIN_NRG_VALUE,
                    KEY_MAX: PRS_MAX_NRG_VALUE,
                },
                "MinStationaryTargetEnergy": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "Min stationary target energy",
                    KEY_LABELFOR: "mise",
                    KEY_DEFAULT: PRS_MIN_NRG_VALUE,
                    KEY_MIN: PRS_MIN_NRG_VALUE,
                    KEY_MAX: PRS_MAX_NRG_VALUE,
                },
            },
        ]
    },
    "network": {
        KEY_TITLE: "Network",
        KEY_GROUPS: [
            {
                KEY_TITLE: "Web interface login",
                KEY_EXPLANATION: "When password is empty, the web interface will be accessible without a login (NOT recommended!).",
                "WebAuthUsername": {
                    KEY_TYPE: VAL_TYPE_STRING,
                    KEY_LABEL: "User",
                    KEY_LABELFOR: "waun",
                    KEY_DEFAULT: "admin",
                    KEY_ALLOW_EMPTY: False
                },
                "WebAuthPassword": {
                    KEY_TYPE: VAL_TYPE_PASSWORD,
                    KEY_LABEL: "Password",
                    KEY_LABELFOR: "wapw",
                    KEY_DEFAULT: "lamp",
                    KEY_ALLOW_EMPTY: True
                },
            },
            {
                KEY_TITLE: "WiFi Access",
                KEY_EXPLANATION: "When SSID is empty, the lamp will not try to connect to a WiFi network. The lamp will boot into Access Point Mode when the credentials are invalid.",
                "WifiStaSsid": {
                    KEY_TYPE: VAL_TYPE_STRING,
                    KEY_LABEL: "WiFi network name (SSID)",
                    KEY_LABELFOR: "wsss",
                    KEY_DEFAULT: "",
                    KEY_ALLOW_EMPTY: True
                },
                "WifiStaPassphrase": {
                    KEY_TYPE: VAL_TYPE_PASSWORD,
                    KEY_LABEL: "Password",
                    KEY_LABELFOR: "wspa",
                    KEY_DEFAULT: "",
                    KEY_ALLOW_EMPTY: True
                },
                "WifiHostname": {
                    KEY_TYPE: VAL_TYPE_STRING,
                    KEY_LABEL: "Hostname (max len 32)",
                    KEY_LABELFOR: "whon",
                    KEY_DEFAULT: "esp32LEDStrip",
                    KEY_ALLOW_EMPTY: False
                },
            },
            {
                KEY_TITLE: "Access Point",
                KEY_EXPLANATION: "To access the access point without a password, leave Password empty.",
                "WifiApSsid": {
                    KEY_TYPE: VAL_TYPE_STRING,
                    KEY_LABEL: "Access Point network name (SSID)",
                    KEY_LABELFOR: "wass",
                    KEY_DEFAULT: "esp32LEDStrip",
                    KEY_ALLOW_EMPTY: False
                },
                "WifiApPassphrase": {
                    KEY_TYPE: VAL_TYPE_PASSWORD,
                    KEY_LABEL: "Password",
                    KEY_LABELFOR: "wapa",
                    KEY_DEFAULT: "",
                    KEY_ALLOW_EMPTY: True
                },
                "WifiAPpIPv4Address": {
                    KEY_TYPE: VAL_TYPE_IPV4,
                    KEY_LABEL: "IPv4 address",
                    KEY_LABELFOR: "waip",
                    KEY_DEFAULT: "192.168.72.12",
                    KEY_ALLOW_EMPTY: False
                },
                "WifiAPpIPv4Netmask": {
                    KEY_TYPE: VAL_TYPE_IPV4,
                    KEY_LABEL: "IPv4 net mask",
                    KEY_LABELFOR: "wanm",
                    KEY_DEFAULT: "255.255.255.0",
                    KEY_ALLOW_EMPTY: False
                },
            },
            {
                KEY_TITLE: "MQTT",
                "mqtt1_RENAME_ME!": {
                    KEY_TYPE: VAL_TYPE_STRING,
                    KEY_LABEL: "Server address",
                    KEY_LABELFOR: "mqsv",
                    KEY_DEFAULT: "",
                    KEY_ALLOW_EMPTY: True
                },
                "mqtt2_RENAME_ME!": {
                    KEY_TYPE: VAL_TYPE_STRING,
                    KEY_LABEL: "Username",
                    KEY_LABELFOR: "mqus",
                    KEY_DEFAULT: "",
                    KEY_ALLOW_EMPTY: True
                },
                "mqtt3_RENAME_ME!": {
                    KEY_TYPE: VAL_TYPE_PASSWORD,
                    KEY_LABEL: "Password",
                    KEY_LABELFOR: "mqpw",
                    KEY_DEFAULT: "",
                    KEY_ALLOW_EMPTY: True
                },
            },
        ]


    },
    "system": {
        KEY_TITLE: "System",
        KEY_GROUPS: [
            {
                KEY_TITLE: "Brightness settings",
                "TransitionDurationMs": {
                    KEY_TYPE: VAL_TYPE_UINT16,
                    KEY_LABEL: "Transition duration (millisecs)",
                    KEY_LABELFOR: "ptdm",
                    KEY_DEFAULT: LIGHT_DEF_TRANSITION_TIME,
                    KEY_MIN: LIGHT_MIN_TRANSITION_TIME,
                    KEY_MAX: LIGHT_MAX_TRANSITION_TIME,
                },
                "system2_RENAME_ME!": {
                    KEY_TYPE: VAL_TYPE_UINT8,
                    KEY_LABEL: "In-/Decrease per step",
                    KEY_LABELFOR: "stbr",
                    KEY_DEFAULT: LIGHT_DEF_BRIGHTNESS_STEP,
                    KEY_MIN: LIGHT_MIN_BRIGHTNESS_STEP,
                    KEY_MAX: LIGHT_MAX_BRIGHTNESS_STEP,
                },
                # static const char *PrefSaveAsPreference = "sapr";
                # static const char *PrefSetLampState = "slst";
            },
        ]
    },
}


check_labels: list[str] = []


def gen_html_top() -> None:
    """
    generate html lead in
    """
    print("""<!DOCTYPE html>
<html lang="en">
<!-- -->
<!-- THIS FILE IS GENERATED BY generate_html.py-->
<!-- -->
<!-- MAKE ANY CHANGES THERE, OTHERWISE THEY WILL BE LOST ON THE NEXT RUN!-->
<!-- -->
<head>
    <title>ESP32 LED Strip Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="style.css">
    <link rel="icon" href="data:,">
</head>

<body>
    <script>
        function backButton() {
            setTimeout(function () { window.open("index.html", "_self"); }, 300);
        }
    </script>
""")


def gen_html_bottom() -> None:
    """
    generate html bottom
    """
    print("""    <button onclick="backButton()">Back</button>
</body>
</html>
""")


def check_label_for_uniqueness(label_for: str) -> None:
    """
    check uniqueness of a 'label for'
    """
    if label_for != "ERROR" and label_for in check_labels:
        print(f"ERROR, non-unique label for {label_for}")
    elif len(label_for) == 0:
        print(f"ERROR, empty label for {label_for}")
    else:
        check_labels.append(label_for)


def gen_group_member(member) -> None:
    """
    generate group member
    """
    label_for: str = member.get(KEY_LABELFOR, "ERROR")
    check_label_for_uniqueness(label_for)

    val_type: str = member.get(KEY_TYPE, "ERROR")

    if val_type == "ERROR":
        print("ERROR, value type not set")
        return

    if val_type not in VAL_TYPES:
        print(f"ERROR: unknown value type {val_type}")
        return

    val_def: str = member.get(KEY_DEFAULT, "ERROR")
    if val_def == "ERROR":
        print("ERROR, default value not set")
        return

    label: str = member.get(KEY_LABEL, "ERROR")
    if label == "ERROR":
        print("ERROR, label not set")
        return

    indent1: str = "        "
    indent2: str = indent1 + "    "

    is_required: bool = member.get(KEY_ALLOW_EMPTY, True) is False
    required: str = " required" if is_required else ""
    mark_label: str = ' class="required"' if is_required else ""

    hover_text = f"default: {val_def}"

    print(f'{indent1}<div title="{hover_text}"><form action="/v1/post" method="post">')
    print(f'{indent2}<label{mark_label} for="{label_for}">{label}: </label>')

    append_value: bool = True
    if val_type == VAL_TYPE_BOOL:
        input_type: str = 'checkbox'
        additional: str = ' checked=checked' if val_def == str(True) else ''
        append_value = False

    elif val_type == VAL_TYPE_STRING:
        input_type = 'text'
        additional = ''

    elif val_type == VAL_TYPE_PASSWORD:
        input_type = 'password'
        additional = 'autocomplete="off" spellcheck="false"'
        append_value = False

    elif val_type == VAL_TYPE_IPV4:
        input_type = 'text'
        pattern = r"^((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$"
        additional = f' minlength="7" maxlength="15" size="15" pattern="{pattern}"'

    else:
        input_type = 'number'
        val_min: str = member.get(KEY_MIN, "ERROR")
        val_max: str = member.get(KEY_MAX, "ERROR")
        additional = f' min="{val_min}" max="{val_max}" step="1" inputmode="decimal"'

    base = f'input type="{input_type}"{required} id="{label_for}" name="{label_for}"'
    value = f' value="{val_def}"' if append_value else ''
    print(f'{indent2}<{base}{additional}{value}>')
    print(f'{indent2}<button name="b{label_for}" value="1">Set</button>')
    print(f'{indent1}</form></div>')


def gen_group(group) -> None:
    """
    generate one group
    """
    indent: str = "    "
    indent2: str = indent + "    "
    print(f'{indent}<div class="group">')
    group_title: str = ""
    group_explanation: str = ""
    group_details: str = ""
    group_members = {}
    for group_key, group_val in group.items():
        if group_key == KEY_EXPLANATION:
            group_explanation = group_val
        elif group_key == KEY_TITLE:
            group_title = group_val
        elif group_key == KEY_DETAILS:
            group_details = group_val
        else:
            group_members.update({group_key: group_val})
    if len(group_title) > 0:
        print(f"{indent2}<h3>{group_title}</h3>")
    if len(group_explanation) > 0:
        print(f"{indent2}<p>{group_explanation}</p>")
    if len(group_details) > 0:
        print(f"{indent2}<p>{group_details}</p>")
    for member in group_members.values():
        gen_group_member(member)
    print(f"{indent}</div>")


def gen_page(page) -> None:
    """
    gen one page
    """
    indent: str = "    "
    page_title: str = page.get(KEY_TITLE, "")
    if len(page_title) > 0:
        print(f'{indent}<div class="category">')
        print(f'{indent}<h2>{page_title}</h2>')
    else:
        print("ERROR: empty page title")
    page_explanation: str = page.get(KEY_EXPLANATION, "")
    if len(page_explanation) > 0:
        print(f"{indent}<p>{page_explanation}</p>")
    page_details: str = page.get(KEY_DETAILS, "")
    if len(page_details) > 0:
        print(f"{indent}<p>{page_details}</p>")
    page_groups = page.get(KEY_GROUPS, [])
    for group in page_groups:
        gen_group(group)
    print(f'{indent}</div>')
    print()


def main() -> None:
    """
    generate
    """

    gen_css()

    gen_html_top()
    print("    <h1>Configuration</h1>")
    print("    <p>Mandatory values are underlined.</p>")
    for page in DEFINITION.values():
        gen_page(page)
    gen_html_bottom()


if __name__ == "__main__":
    main()
