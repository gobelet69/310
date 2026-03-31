C'est une excellente idée de lier les exigences de votre projet directement aux travaux pratiques que vous avez déjà réalisés. Vos trois labos couvrent en fait **l'intégralité** des briques techniques nécessaires pour construire votre automate "Never-lose". 

Voici la cartographie exacte pour savoir où aller chercher vos extraits de code et vos configurations matérielles :

### 📘 Lab 1 : Interfaces de base, Affichage et Capteurs Analogiques
Ce premier labo va constituer le cœur de votre interface utilisateur et de votre "vision" (les capteurs).

* [cite_start]**Boutons (SW1, SW2, SW3) et LEDs (D1 à D4) :** Vous trouverez comment configurer ces entrées/sorties de base (GPIO) dans la section 2 du Lab 1[cite: 140, 142, 144].
* [cite_start]**Écran LCD :** L'affichage des états ("Jump", "Duck") et la mise à jour du score utiliseront les fonctions de type `LCD_Char_...` (comme `LCD_Char_PrintString` et `LCD_Char_PrintNumber`) vues dans la section 3 [cite: 186, 206, 208-210].
* [cite_start]**Clavier (Keyboard) :** Le mappage de deux touches spécifiques pour répliquer le comportement de SW1 et SW2 se fera grâce à la logique de la matrice du clavier et la fonction `keypadScan()` abordées dans la section 4[cite: 227, 231, 246].
* [cite_start]**Photorésistances (Détection d'obstacles et Bonus Jour/Nuit) :** La conversion de la luminosité en une valeur utilisable par le microcontrôleur via l'ADC (Convertisseur Analogique-Digital) et le circuit diviseur de tension est détaillée dans la section 5.2[cite: 322, 324, 327, 339].

### 📙 Lab 2 : Gestion du temps et Génération de son
Ce labo est crucial pour la logique interne du jeu et le retour auditif.

* [cite_start]**Comptage du score (Chronomètre) :** La notion de temps pour incrémenter le score (10 points par seconde) utilisera les Timers vus dans la section 2[cite: 378, 381].
* [cite_start]**Fiabilité du Score (Interruptions) :** Pour que votre score s'incrémente précisément en arrière-plan sans être ralenti par le reste de votre code (comme les lectures de capteurs ou les délais), vous devrez lier votre Timer à une Interruption (ISR), expliquée dans la section 4[cite: 470, 472, 474].
* [cite_start]**Sortie Audio (Bruitages) :** La génération des signaux pour créer les sons de saut et de baisse se fera via le DAC (Convertisseur Digital-Analogique) vu dans la section 3[cite: 421, 425, 437].

### 📕 Lab 3 : Actionneurs et Communication
C'est ici que l'automate agit physiquement et discute avec l'ordinateur.

* [cite_start]**Servomoteurs (Appui sur les touches) :** Le contrôle précis de l'angle de vos deux servomoteurs (pour frapper la barre espace et la flèche du bas) se fera en générant des signaux PWM (Pulse Width Modulation), expliqué dans la section 2[cite: 518, 520, 528].
* [cite_start]**Port Série (Communication UART) :** La réception des commandes PC ("jump", "duck") et l'envoi des états du PSoC vers le PC utiliseront le protocole UART vu dans la section 3[cite: 563, 566, 568]. [cite_start]Vous y retrouverez les méthodes pour envoyer des données (`UART_PutString`) [cite: 612, 630] [cite_start]et configurer une interruption pour la réception des données (`rxData = UART_ReadRxData()`)[cite: 638, 639, 661].

---

Vous avez tout le puzzle entre les mains ! Souhaitez-vous que l'on commence par revoir le code d'un module en particulier, par exemple la configuration du **Timer avec interruption** pour s'assurer que votre score fonctionnera parfaitement de manière indépendante ?