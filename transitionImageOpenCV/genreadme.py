import json

# Données des transitions et leurs sous-options
transitions = [
    {
        "name": "slider",
        "description": "Effet de glissement où une image glisse sur l'autre dans une direction donnée.",
        "sub_options": [
            {"name": "direction", "type": "string", "default": "\"right-to-left\"",
             "values": ["\"right-to-left\", \"rtl\", \"right\"", "\"left-to-right\", \"ltr\", \"left\"",
                        "\"top-to-bottom\", \"ttb\", \"top\"", "\"bottom-to-top\", \"btt\", \"bottom\""],
             "description": "Direction du glissement."}
        ]
    },
    {
        "name": "slideright",
        "description": "Version de compatibilité pour un glissement de droite à gauche (équivalent à slider avec direction=\"right-to-left\").",
        "sub_options": []
    },
    {
        "name": "fade",
        "description": "Fondu enchaîné (cross-dissolve) où l'opacité de la première image diminue tandis que celle de la deuxième augmente.",
        "sub_options": []
    },
    {
        "name": "appearright",
        "description": "Transition où la deuxième image \"apparaît\" depuis la droite (probablement un effet de glissement partiel ou spécifique).",
        "sub_options": []
    },
    {
        "name": "wipe",
        "description": "Effet de balayage où une frontière droite se déplace pour révéler la deuxième image.",
        "sub_options": [
            {"name": "direction", "type": "string", "default": "\"left-to-right\"",
             "values": ["\"left-to-right\", \"ltr\", \"left\"", "\"right-to-left\", \"rtl\", \"right\"",
                        "\"top-to-bottom\", \"ttb\", \"top\"", "\"bottom-to-top\", \"btt\", \"bottom\""],
             "description": "Direction du balayage."}
        ]
    },
    {
        "name": "barndoor",
        "description": "Effet de \"porte de grange\" où l'image se divise en deux parties qui s'écartent.",
        "sub_options": [
            {"name": "orientation", "type": "string", "default": "\"horizontal\"",
             "values": ["\"horizontal\"", "\"vertical\""],
             "description": "Orientation de l'ouverture."}
        ]
    },
    {
        "name": "radial",
        "description": "Transition radiale où la deuxième image est révélée à partir d'un point central.",
        "sub_options": [
            {"name": "center_x", "type": "int", "default": "size.width / 2", "description": "Coordonnée x du centre."},
            {"name": "center_y", "type": "int", "default": "size.height / 2", "description": "Coordonnée y du centre."}
        ]
    },
    {
        "name": "pie",
        "description": "Transition en forme de \"tarte\" où la deuxième image est révélée en arc de cercle.",
        "sub_options": [
            {"name": "center_x", "type": "int", "default": "size.width / 2", "description": "Coordonnée x du centre."},
            {"name": "center_y", "type": "int", "default": "size.height / 2", "description": "Coordonnée y du centre."},
            {"name": "start_angle", "type": "double", "default": "-90.0", "description": "Angle de départ (en degrés)."},
            {"name": "direction", "type": "string", "default": "\"ccw\"",
             "values": ["\"ccw\" (sens anti-horaire)", "Autre (sens horaire)"],
             "description": "Direction de rotation."}
        ]
    },
    {
        "name": "pieadvanced (ou piesweep)",
        "description": "Version avancée de la transition \"pie\" avec des paramètres supplémentaires pour le contrôle du rayon et de l'angle.",
        "sub_options": [
            {"name": "center_x", "type": "int", "default": "size.width / 2", "description": "Coordonnée x du centre."},
            {"name": "center_y", "type": "int", "default": "size.height / 2", "description": "Coordonnée y du centre."},
            {"name": "start_angle", "type": "double", "default": "-90.0", "description": "Angle de départ (en degrés)."},
            {"name": "direction", "type": "string", "default": "\"ccw\"",
             "values": ["\"ccw\" (sens anti-horaire)", "Autre (sens horaire)"],
             "description": "Direction de rotation."},
            {"name": "r0_frac", "type": "double", "default": "0.0", "description": "Fraction du rayon initial."},
            {"name": "r1_frac", "type": "double", "default": "1.0", "description": "Fraction du rayon final."},
            {"name": "sweep_deg", "type": "double", "default": "360.0", "description": "Angle de balayage (en degrés)."}
        ]
    },
    {
        "name": "zoom",
        "description": "Transition de zoom (grossissement ou réduction) pour révéler la deuxième image.",
        "sub_options": [
            {"name": "mode", "type": "string", "default": "\"in\"",
             "values": ["\"in\" (zoom avant)", "\"out\" (zoom arrière)"],
             "description": "Type de zoom."}
        ]
    },
    {
        "name": "blur",
        "description": "Transition basée sur un flou progressif.",
        "sub_options": []
    },
    {
        "name": "checkerboard (ou damier)",
        "description": "Transition en damier où la deuxième image est révélée par blocs carrés.",
        "sub_options": [
            {"name": "squares", "type": "int", "default": "8", "description": "Nombre de carrés par côté (si rows et cols non spécifiés)."},
            {"name": "rows", "type": "int", "default": "Valeur de squares", "description": "Nombre de lignes de carrés."},
            {"name": "cols", "type": "int", "default": "Valeur de squares", "description": "Nombre de colonnes de carrés."},
            {"name": "stepwise", "type": "bool", "default": "true", "description": "Si true, les carrés apparaissent un par un."},
            {"name": "seed", "type": "unsigned", "default": "1234", "description": "Graine pour la génération aléatoire (si applicable)."}
        ]
    },
    {
        "name": "movingbars (ou bars)",
        "description": "Transition avec des barres mobiles révélant la deuxième image.",
        "sub_options": [
            {"name": "axis", "type": "string", "default": "\"horizontal\"",
             "values": ["\"horizontal\"", "\"vertical\""],
             "description": "Orientation des barres."},
            {"name": "count", "type": "int", "default": "16", "description": "Nombre de barres."},
            {"name": "direction", "type": "string", "default": "\"bottom\"", "description": "Direction du mouvement."},
            {"name": "speed_min", "type": "double", "default": "0.5", "description": "Vitesse minimale des barres."},
            {"name": "speed_max", "type": "double", "default": "1.5", "description": "Vitesse maximale des barres."},
            {"name": "seed", "type": "unsigned", "default": "42", "description": "Graine pour la génération aléatoire."}
        ]
    },
    {
        "name": "interleave",
        "description": "Transition avec des rectangles entrelacés révélant la deuxième image.",
        "sub_options": [
            {"name": "bands", "type": "int", "default": "10", "description": "Nombre de bandes entrelacées."}
        ]
    },
    {
        "name": "randomcircles",
        "description": "Transition où des cercles aléatoires révèlent la deuxième image.",
        "sub_options": [
            {"name": "count", "type": "int", "default": "20", "description": "Nombre de cercles."},
            {"name": "seed", "type": "unsigned", "default": "12345", "description": "Graine pour la génération aléatoire."}
        ]
    },
    {
        "name": "randomsquares",
        "description": "Transition où des carrés aléatoires révèlent la deuxième image.",
        "sub_options": [
            {"name": "count", "type": "int", "default": "20", "description": "Nombre de carrés."},
            {"name": "seed", "type": "unsigned", "default": "12345", "description": "Graine pour la génération aléatoire."}
        ]
    },
    {
        "name": "blinds",
        "description": "Transition en \"persiennes\" où des bandes révèlent la deuxième image.",
        "sub_options": [
            {"name": "axis", "type": "string", "default": "\"vertical\"",
             "values": ["\"vertical\"", "\"horizontal\""],
             "description": "Orientation des persiennes."},
            {"name": "count", "type": "int", "default": "16", "description": "Nombre de persiennes."},
            {"name": "direction", "type": "string", "default": "\"left\"", "description": "Direction du mouvement."},
            {"name": "wave_amplitude", "type": "double", "default": "0.0", "description": "Amplitude de l'effet de vague (optionnel)."},
            {"name": "wave_phase", "type": "double", "default": "0.0", "description": "Phase de l'effet de vague (optionnel)."}
        ]
    },
    {
        "name": "checkerboardanimated (ou checkerboard_anim, checkerboard-animated)",
        "description": "Transition en damier animé où les carrés apparaissent selon un ordre spécifique.",
        "sub_options": [
            {"name": "squares", "type": "int", "default": "10", "description": "Nombre de carrés par côté (si rows et cols non spécifiés)."},
            {"name": "rows", "type": "int", "default": "Valeur de squares", "description": "Nombre de lignes de carrés."},
            {"name": "cols", "type": "int", "default": "Valeur de squares", "description": "Nombre de colonnes de carrés."},
            {"name": "order", "type": "string", "default": "\"row\"",
             "values": ["\"row\"", "\"col\", \"column\", \"columns\"", "\"diag\", \"diagonal\"",
                        "\"invdiag\", \"invdiagonal\"", "\"random\""],
             "description": "Ordre d'apparition des carrés."},
            {"name": "seed", "type": "unsigned", "default": "1234", "description": "Graine pour l'ordre aléatoire."}
        ]
    }
]

# Paramètres communs
common_parameters = [
    {"name": "duration", "type": "double", "default": "3.0", "description": "Durée de la transition (en secondes)."},
    {"name": "fps", "type": "double", "default": "Valeur de defaultFps (ou 30.0 si defaultFps <= 0)", "description": "Nombre d'images par seconde (frames per second)."},
    {"name": "easing", "type": "string", "default": "\"linear\"", "description": "Type de courbe d'animation pour la transition."},
    {"name": "mask_blur", "type": "object", "default": "Aucun (optionnel)", "description": "Options de flou appliquées au masque de transition."}
]

mask_blur_parameters = [
    {"name": "type", "type": "string", "default": "\"none\"", "description": "Type de flou (par exemple, \"gaussian\", \"none\")."},
    {"name": "ksize", "type": "int", "default": "0", "description": "Taille du noyau de flou (doit être impair pour cv::GaussianBlur)."},
    {"name": "sigma", "type": "double", "default": "0.0", "description": "Écart-type pour le flou gaussien."},
    {"name": "opacitychange", "type": "bool", "default": "false", "description": "Indique si l'opacité du masque change pendant la transition."}
]

easing_options = [
    "ease-in", "ease-out", "ease-in-out", "ease-in-bounce", "ease-out-bounce",
    "ease-in-elastic", "ease-out-elastic", "ease-in-circ", "ease-out-circ",
    "ease-inout-circ", "ease-in-quint", "ease-out-quint", "ease-inout-quint"
]

# Fonction pour générer le Markdown
def generate_markdown():
    markdown = "# Types de transitions et leurs sous-options\n\n"
    markdown += "La fonction `createTransitionFromJson` prend en charge plusieurs types de transitions, chacune avec des paramètres spécifiques définis dans le JSON. Les sous-options incluent des paramètres communs (comme `duration`, `fps`, `easing`, `mask_blur`) et des paramètres spécifiques à chaque type de transition.\n\n"

    # Paramètres communs
    markdown += "## Paramètres communs à toutes les transitions\n\n"
    markdown += "Ces paramètres s'appliquent à **toutes** les transitions, sauf indication contraire :\n\n"
    markdown += "| Paramètre | Description | Type | Valeur par défaut |\n"
    markdown += "|-----------|-------------|------|-------------------|\n"
    for param in common_parameters:
        markdown += f"| `{param['name']}` | {param['description']} | `{param['type']}` | `{param['default']}` |\n"
    markdown += "\n"

    # Sous-options de mask_blur
    markdown += "#### Sous-options de `mask_blur`\n\n"
    markdown += "Le paramètre `mask_blur` est un objet JSON optionnel qui peut être utilisé pour ajouter un effet de flou au masque de transition. Ses sous-options sont :\n\n"
    markdown += "| Sous-option | Description | Type | Valeur par défaut |\n"
    markdown += "|-------------|-------------|------|-------------------|\n"
    for param in mask_blur_parameters:
        markdown += f"| `{param['name']}` | {param['description']} | `{param['type']}` | `{param['default']}` |\n"
    markdown += "\n"

    # Valeurs possibles pour easing
    markdown += "#### Valeurs possibles pour `easing`\n\n"
    markdown += "La liste des courbes d'animation (`easing`) est définie dans la méthode `TransitionBetween::getEasing()` :\n\n"
    for easing in easing_options:
        markdown += f"- `{easing}`\n"
    markdown += "\n"

    # Transitions spécifiques
    markdown += "## Transitions spécifiques et leurs sous-options\n\n"
    markdown += "Voici chaque type de transition avec ses sous-options spécifiques, basées sur le code de `transitions.cpp`. Les types sont listés dans l'ordre de la méthode `TransitionBetween::getTransitiontring()`.\n\n"

    for transition in transitions:
        markdown += f"#### {transition['name']}\n"
        markdown += f"{transition['description']}\n\n"
        if transition["sub_options"]:
            markdown += "| Sous-option | Description | Type | Valeur par défaut |\n"
            markdown += "|-------------|-------------|------|-------------------|\n"
            for opt in transition["sub_options"]:
                values = "<br>".join(opt.get("values", [])) if "values" in opt else "-"
                markdown += f"| `{opt['name']}` | {opt['description']} | `{opt['type']}` | `{opt['default']}` |\n"
                if values != "-":
                    markdown += f"| | Valeurs possibles : <br>{values} | | |\n"
            markdown += "\n"
        else:
            markdown += "| Sous-option | Description | Type | Valeur par défaut |\n"
            markdown += "|-------------|-------------|------|-------------------|\n"
            markdown += "| Aucune | Utilise uniquement les paramètres communs et `mask_blur`. | - | - |\n\n"

    # Exemple JSON
    markdown += "## Exemple d'utilisation (JSON)\n\n"
    markdown += "Voici un exemple de configuration JSON pour une transition `slider` :\n\n"
    markdown += "```json\n"
    markdown += json.dumps({
        "type": "slider",
        "duration": 2.0,
        "fps": 24.0,
        "easing": "ease-in-out",
        "direction": "left-to-right",
        "mask_blur": {
            "type": "gaussian",
            "ksize": 5,
            "sigma": 1.0,
            "opacitychange": True
        }
    }, indent=2)
    markdown += "\n```\n"

    return markdown

# Écrire le Markdown dans un fichier
def write_markdown_to_file(filename="README.md"):
    markdown_content = generate_markdown()
    with open(filename, "w", encoding="utf-8") as f:
        f.write(markdown_content)
    print(f"Fichier Markdown généré : {filename}")

if __name__ == "__main__":
    write_markdown_to_file()
