# Types de transitions et leurs sous-options

La fonction `createTransitionFromJson` prend en charge plusieurs types de transitions, chacune avec des paramètres spécifiques définis dans le JSON. Les sous-options incluent des paramètres communs (comme `duration`, `fps`, `easing`, `mask_blur`) et des paramètres spécifiques à chaque type de transition.

## Paramètres communs à toutes les transitions

Ces paramètres s'appliquent à **toutes** les transitions, sauf indication contraire :

| Paramètre | Description | Type | Valeur par défaut |
|-----------|-------------|------|-------------------|
| `duration` | Durée de la transition (en secondes). | `double` | `3.0` |
| `fps` | Nombre d'images par seconde (frames per second). | `double` | `Valeur de defaultFps (ou 30.0 si defaultFps <= 0)` |
| `easing` | Type de courbe d'animation pour la transition. | `string` | `"linear"` |
| `mask_blur` | Options de flou appliquées au masque de transition. | `object` | `Aucun (optionnel)` |

#### Sous-options de `mask_blur`

Le paramètre `mask_blur` est un objet JSON optionnel qui peut être utilisé pour ajouter un effet de flou au masque de transition. Ses sous-options sont :

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `type` | Type de flou (par exemple, "gaussian", "none"). | `string` | `"none"` |
| `ksize` | Taille du noyau de flou (doit être impair pour cv::GaussianBlur). | `int` | `0` |
| `sigma` | Écart-type pour le flou gaussien. | `double` | `0.0` |
| `opacitychange` | Indique si l'opacité du masque change pendant la transition. | `bool` | `false` |

#### Valeurs possibles pour `easing`

La liste des courbes d'animation (`easing`) est définie dans la méthode `TransitionBetween::getEasing()` :

- `ease-in`
- `ease-out`
- `ease-in-out`
- `ease-in-bounce`
- `ease-out-bounce`
- `ease-in-elastic`
- `ease-out-elastic`
- `ease-in-circ`
- `ease-out-circ`
- `ease-inout-circ`
- `ease-in-quint`
- `ease-out-quint`
- `ease-inout-quint`

## Transitions spécifiques et leurs sous-options

Voici chaque type de transition avec ses sous-options spécifiques, basées sur le code de `transitions.cpp`. Les types sont listés dans l'ordre de la méthode `TransitionBetween::getTransitiontring()`.

#### slider
Effet de glissement où une image glisse sur l'autre dans une direction donnée.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `direction` | Direction du glissement. | `string` | `"right-to-left"` |
| | Valeurs possibles : <br>"right-to-left", "rtl", "right"<br>"left-to-right", "ltr", "left"<br>"top-to-bottom", "ttb", "top"<br>"bottom-to-top", "btt", "bottom" | | |

#### slideright
Version de compatibilité pour un glissement de droite à gauche (équivalent à slider avec direction="right-to-left").

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| Aucune | Utilise uniquement les paramètres communs et `mask_blur`. | - | - |

#### fade
Fondu enchaîné (cross-dissolve) où l'opacité de la première image diminue tandis que celle de la deuxième augmente.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| Aucune | Utilise uniquement les paramètres communs et `mask_blur`. | - | - |

#### appearright
Transition où la deuxième image "apparaît" depuis la droite (probablement un effet de glissement partiel ou spécifique).

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| Aucune | Utilise uniquement les paramètres communs et `mask_blur`. | - | - |

#### wipe
Effet de balayage où une frontière droite se déplace pour révéler la deuxième image.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `direction` | Direction du balayage. | `string` | `"left-to-right"` |
| | Valeurs possibles : <br>"left-to-right", "ltr", "left"<br>"right-to-left", "rtl", "right"<br>"top-to-bottom", "ttb", "top"<br>"bottom-to-top", "btt", "bottom" | | |

#### barndoor
Effet de "porte de grange" où l'image se divise en deux parties qui s'écartent.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `orientation` | Orientation de l'ouverture. | `string` | `"horizontal"` |
| | Valeurs possibles : <br>"horizontal"<br>"vertical" | | |

#### radial
Transition radiale où la deuxième image est révélée à partir d'un point central.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `center_x` | Coordonnée x du centre. | `int` | `size.width / 2` |
| `center_y` | Coordonnée y du centre. | `int` | `size.height / 2` |

#### pie
Transition en forme de "tarte" où la deuxième image est révélée en arc de cercle.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `center_x` | Coordonnée x du centre. | `int` | `size.width / 2` |
| `center_y` | Coordonnée y du centre. | `int` | `size.height / 2` |
| `start_angle` | Angle de départ (en degrés). | `double` | `-90.0` |
| `direction` | Direction de rotation. | `string` | `"ccw"` |
| | Valeurs possibles : <br>"ccw" (sens anti-horaire)<br>Autre (sens horaire) | | |

#### pieadvanced (ou piesweep)
Version avancée de la transition "pie" avec des paramètres supplémentaires pour le contrôle du rayon et de l'angle.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `center_x` | Coordonnée x du centre. | `int` | `size.width / 2` |
| `center_y` | Coordonnée y du centre. | `int` | `size.height / 2` |
| `start_angle` | Angle de départ (en degrés). | `double` | `-90.0` |
| `direction` | Direction de rotation. | `string` | `"ccw"` |
| | Valeurs possibles : <br>"ccw" (sens anti-horaire)<br>Autre (sens horaire) | | |
| `r0_frac` | Fraction du rayon initial. | `double` | `0.0` |
| `r1_frac` | Fraction du rayon final. | `double` | `1.0` |
| `sweep_deg` | Angle de balayage (en degrés). | `double` | `360.0` |

#### zoom
Transition de zoom (grossissement ou réduction) pour révéler la deuxième image.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `mode` | Type de zoom. | `string` | `"in"` |
| | Valeurs possibles : <br>"in" (zoom avant)<br>"out" (zoom arrière) | | |

#### blur
Transition basée sur un flou progressif.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| Aucune | Utilise uniquement les paramètres communs et `mask_blur`. | - | - |

#### checkerboard (ou damier)
Transition en damier où la deuxième image est révélée par blocs carrés.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `squares` | Nombre de carrés par côté (si rows et cols non spécifiés). | `int` | `8` |
| `rows` | Nombre de lignes de carrés. | `int` | `Valeur de squares` |
| `cols` | Nombre de colonnes de carrés. | `int` | `Valeur de squares` |
| `stepwise` | Si true, les carrés apparaissent un par un. | `bool` | `true` |
| `seed` | Graine pour la génération aléatoire (si applicable). | `unsigned` | `1234` |

#### movingbars (ou bars)
Transition avec des barres mobiles révélant la deuxième image.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `axis` | Orientation des barres. | `string` | `"horizontal"` |
| | Valeurs possibles : <br>"horizontal"<br>"vertical" | | |
| `count` | Nombre de barres. | `int` | `16` |
| `direction` | Direction du mouvement. | `string` | `"bottom"` |
| `speed_min` | Vitesse minimale des barres. | `double` | `0.5` |
| `speed_max` | Vitesse maximale des barres. | `double` | `1.5` |
| `seed` | Graine pour la génération aléatoire. | `unsigned` | `42` |

#### interleave
Transition avec des rectangles entrelacés révélant la deuxième image.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `bands` | Nombre de bandes entrelacées. | `int` | `10` |

#### randomcircles
Transition où des cercles aléatoires révèlent la deuxième image.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `count` | Nombre de cercles. | `int` | `20` |
| `seed` | Graine pour la génération aléatoire. | `unsigned` | `12345` |

#### randomsquares
Transition où des carrés aléatoires révèlent la deuxième image.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `count` | Nombre de carrés. | `int` | `20` |
| `seed` | Graine pour la génération aléatoire. | `unsigned` | `12345` |

#### blinds
Transition en "persiennes" où des bandes révèlent la deuxième image.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `axis` | Orientation des persiennes. | `string` | `"vertical"` |
| | Valeurs possibles : <br>"vertical"<br>"horizontal" | | |
| `count` | Nombre de persiennes. | `int` | `16` |
| `direction` | Direction du mouvement. | `string` | `"left"` |
| `wave_amplitude` | Amplitude de l'effet de vague (optionnel). | `double` | `0.0` |
| `wave_phase` | Phase de l'effet de vague (optionnel). | `double` | `0.0` |

#### checkerboardanimated (ou checkerboard_anim, checkerboard-animated)
Transition en damier animé où les carrés apparaissent selon un ordre spécifique.

| Sous-option | Description | Type | Valeur par défaut |
|-------------|-------------|------|-------------------|
| `squares` | Nombre de carrés par côté (si rows et cols non spécifiés). | `int` | `10` |
| `rows` | Nombre de lignes de carrés. | `int` | `Valeur de squares` |
| `cols` | Nombre de colonnes de carrés. | `int` | `Valeur de squares` |
| `order` | Ordre d'apparition des carrés. | `string` | `"row"` |
| | Valeurs possibles : <br>"row"<br>"col", "column", "columns"<br>"diag", "diagonal"<br>"invdiag", "invdiagonal"<br>"random" | | |
| `seed` | Graine pour l'ordre aléatoire. | `unsigned` | `1234` |

## Exemple d'utilisation (JSON)

Voici un exemple de configuration JSON pour une transition `slider` :

```json
{
  "type": "slider",
  "duration": 2.0,
  "fps": 24.0,
  "easing": "ease-in-out",
  "direction": "left-to-right",
  "mask_blur": {
    "type": "gaussian",
    "ksize": 5,
    "sigma": 1.0,
    "opacitychange": true
  }
}
```
