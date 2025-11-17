# Instructions

Pour pouvoir compiler le projet sur votre ordinateur| il vous faut installer les librairies flex et bison.

- Sur linux (ubuntu) cela se fait par la commande :
  
  sudo apt-get install flex bison
  
  (ou équivalent sur d'autres distributions)

- Sur mac ces librairies sont installées en standard. Si ce n'est pas le cas pour vous| cela peut s'installer via homebrew (cf https://brew.sh/index_fr)

  brew install flex bison

- sur windows  
    Utilisez une WSL.

# Compte rendu

Lors de cette apnée j'ai réalisé une grammaire et son analyseur syntaxique ainsi qu'un interpretreur de calcul. 
La particularité de cette apnée est que je devais calculer la puissance et le nombre de combinaison k parmis n.  
Sa difficulté résidait dans le fait que la puissance soit associative à droite et non à gauche comme la majorité des opérations de calcul et le calcul de la combinaison qui n'est pas trivial comme les autres.

Dans un 1er temps j'ai d'abord trouvé une grammaire, principalement par tatonnement. J'identifiais un besoin de la grammaire (par exemple: chaque ligne fini par un ;) puis j'essayais de l'integrer à la grammaire, et cela pour chaque étape. 

Ensuite la programmation de l'analyseur syntaxique était plutot simple car quasiment automatique à partir de la table LL. La grammaire étant LL2 et non LL1, nous avions à disposition une fonction pour regarder en avant dans le flux de lexème.

Pour finir l'interpreteur de calcul, j'ai décidé de tout calculer à la volée et de ne pas m'encombrer d'un AST qui aurait été bien plus long à réaliser. Egalement j'ai fait le choix de créer une fonction erreur qui s'occupe d'afficher le token qui pose problème et d'arreter le programme

## Grammaires utilisée pour les différentes étapes

### Grammaire originelle $G_1$

$$
\begin{aligned}
& R_0: \quad S \rightarrow A \\
& R_1: \quad A \rightarrow L ; A \\
& R_2: \quad A \rightarrow \varepsilon \\
& R_3: \quad L \rightarrow V = E \\
& R_4: \quad L \rightarrow E \\
& R_5: \quad V \rightarrow \text{var} \\
& R_6: \quad E \rightarrow E + T \\
& R_7: \quad E \rightarrow E - T \\
& R_8: \quad E \rightarrow T \\
& R_9: \quad T \rightarrow T * G \\
& R_{10}: \quad T \rightarrow T / G \\
& R_{11}: \quad T \rightarrow G \\
& R_{12}: \quad G \rightarrow B \wedge G \\
& R_{13}: \quad G \rightarrow B \\
& R_{14}: \quad B \rightarrow C\wedge B \_ F \\
& R_{15}: \quad B \rightarrow C\_ B \wedge F \\
& R_{16}: \quad B \rightarrow F \\
& R_{17}: \quad F \rightarrow \text{var} \\
& R_{18}: \quad F \rightarrow a \\
& R_{19}: \quad F \rightarrow (L)
\end{aligned}
$$

### Grammaire $G_2$ sans récursivité à gauche

$$
\begin{aligned}
R_0: \quad & S \rightarrow A \\
R_1: \quad & A \rightarrow L ; A \\
R_2: \quad & A \rightarrow \varepsilon \\
R_3: \quad & L \rightarrow V = E \\
R_4: \quad & L \rightarrow E \\
R_5: \quad & V \rightarrow \text{var} \\
R_6: \quad & E \rightarrow T E' \\
R_7: \quad & E \rightarrow V = E \\
R_8: \quad & E' \rightarrow - T E' \\
R_9: \quad & E' \rightarrow + T E' \\
R_{10}: \quad & E' \rightarrow \varepsilon \\
R_{11}: \quad & T \rightarrow G T' \\
R_{12}: \quad & T' \rightarrow * G T' \\
R_{13}: \quad & T' \rightarrow / G T' \\
R_{14}: \quad & T' \rightarrow \varepsilon \\
R_{15}: \quad & G \rightarrow B \wedge G \\
R_{16}: \quad & G \rightarrow B \\
R_{17}: \quad & B \rightarrow C \wedge B \_ F \\
R_{18}: \quad & B \rightarrow C \_ B \wedge F \\
R_{19}: \quad & B \rightarrow F \\
R_{20}: \quad & F \rightarrow - F \\
R_{21}: \quad & F \rightarrow \text{var} \\
R_{22}: \quad & F \rightarrow a \\
R_{23}: \quad & F \rightarrow (L)
\end{aligned}
$$

## Table LL1 calculée
voici la table LL1 (en CSV dans le ficher 1er.csv)


| LL(1) | `;` | `+` | `-` | `/` | `*` | `=` | `^` | `(` | `)` | `a` | `var` | `_` | `C` |`$` |
|-------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-------|-----|-----|----|
| $S$ | | | $R_0$ | | | | | $R_0$ | | $R_0$ | $R_0$ | | $R_0$ | $R_0$ |
| $A$ | $R_2$ | | $R_1$ | | | | | $R_1$ | | $R_1$ | $R_1$ | | $R_1$ | $R_2$ |
| $L$ | | | $R_4$ | | | | | $R_4$ | | $R_4$ | $R_3/R_4$ | | $R_4$ | |
| $V$ | | | | | | | | | | | $R_5$ | | | |
| $E$ | | | $R_6$ | | | | | $R_6$ | | $R_6$ | $R_6/R_7$ | | $R_6$  | |
| $E'$ | $R_{10}$ | $R_9$ | $R_8$ | | | | | | $R_{10}$ | | | | | $R_{10}$ |
| $T$ | | | $R_{11}$ | | | | | $R_{11}$ | | $R_{11}$ | $R_{11}$ | | $R_{11}$ | |
| $T'$ | $R_{14}$ | $R_{14}$ | $R_{14}$ | $R_{13}$ | $R_{12}$ | | | | $R_{14}$ | | | | | $R_{14}$ |
| $G$ | | | $R_{15}/R_{16}$ | | | | | $R_{15}/R_{16}$ | | $R_{15}/R_{16}$ | $R_{15}/R_{16}$ | | $R_{15}/R_{16}/R_{17}$ | |
| $B$ | | | $R_{19}$ | | | | | $R_{19}$ | | $R_{19}$ | $R_{19}$ | | $R_{18}$ | |
| $F$ | | | $R_{20}$ | | | | | $R_{23}$ | | $R_{22}$ | $R_{21}$ | | | |

On peut voir qu'il y a plusieurs conflits notament à G et E que l'on résout en regardant le lexeme suivant
## Difficultés rencontrées

Les principales difficultés rencontrées pendant cette apnée était principalement lié à la construction de la grammaire.Beacoup de tatonnement pour arriver au final à une grammaire qui fonctionne correctement.
Entre chaque grammaire la reconstruction de la table LL1 prenait beacoup de temps et je ne voyais pas forcement directement que la grammaire avait une erreur.

Par exemple dans une itération de ma grammaire
R0 valait S $\Rightarrow$ A; alors que $A \rightarrow L;A $ Et donc il y avait une répétition de ; à la fin.

De même pour le - unaire

Egalement un oubli bête dans la grammaire (le C de la combinaison) m'a fait reprendre une bonne partie de la grammaire.

Enfin la lutte contre les segfault en cas de ; manquant à la fin a été pour moi une assez grande frustration et la solution que j'ai trouvé actuellement ne me satisfait pas vraimant. Le fait de simplement `lookup(2) == NULL` au debut permet d'empêcher efficacement les segfaults mais sans grand lien avec la grammaire.

