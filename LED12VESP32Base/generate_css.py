"""
py code to generate the css file
"""


def gen_css() -> None:
    """
    generate the css file
    """

    css_content = """html {
    font-family: Arial;
    display: inline-block;
    text-align: left;
}

h1 {
    font-size: 2.0rem;
    color: rgb(250, 225, 210);
}

h2 {
    font-size: 1.5rem;
    color: rgb(250, 225, 210);
}

h3 {
    font-size: 1.2rem;
    color: rgb(250, 225, 210);
}

p {
    font-size: 1.0rem;
    color: rgb(225, 225, 225);
}

label {
    font-size: 125%;
    color: rgb(225, 225, 225);
    width: 40%;
    display: inline-block;
}

input {
    font-size: 1.0rem;
    color: #000000;
    width: 40%;
    display: inline-block;
    &:invalid {
        border: 6px solid red;
    }
}

button {
    padding: .4em .8em;
    background: #08173f;
    border: thin solid #1c4eda;
    color: white;
    text-shadow: 0 -.05em .05em #333;
    font-size: 125%;
    line-height: 1.5;
}

body {
    max-width: 600px;
    margin: 0px auto;
    padding-bottom: 25px;
    background-color: #133592;
}

.group {
  background-color: #1640b4;
}

.required {
    text-decoration: underline;
}"""

    with open(file='style.css', mode='w', encoding='utf-8') as f:
        f.write(css_content)
