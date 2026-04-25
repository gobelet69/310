### Boutons et Contrôles
- [x] **SW1** : Activer le servomoteur de la barre d'espace (saut) lors de l'appui.
- [x] **SW2** : Activer le servomoteur de la flèche du bas (baisse) lors de l'appui.
- [x] **SW3** : Réinitialiser le score à zéro lors de l'appui.
- [x] **Clavier du microcontrôleur** : Configurer deux touches pour reproduire les mêmes comportements que SW1 et SW2.

### Logique de Jeu et Score
- [x] **Démarrage du score** : L'incrémentation du score démarre au premier appui sur la barre d'espace (après un reset à zéro).
- [x] **Incrémentation** : Faire augmenter le score à une vitesse de 10 points par seconde.

### Sorties Visuelles (LEDs & LCD)
- [x] **LED 1 & LED 2** : Les allumer lorsque le dinosaure saute.
- [x] **LED 3 & LED 4** : Les allumer lorsque le dinosaure se baisse.
- [x] **Écran LCD (Partie 1)** : Afficher "Jump" lors d'un saut et "Duck" lors d'une baisse.
- [x] **Écran LCD (Partie 2)** : Afficher le score de la partie.

### Sortie Audio
- [x] **Son de saut** : Émettre un son spécifique quand le dinosaure saute.
- [x] **Son de baisse** : Émettre un son différent quand le dinosaure se baisse.

### Capteurs (Photorésistances)
- [x] **Capteur Sol** : Configurer une photorésistance pour détecter les obstacles au sol.
- [x] **Capteur Air** : Configurer la deuxième photorésistance pour détecter les obstacles volants.

### Communication Série (Port Série)
- [x] **Réception PC -> Carte** : Détecter les commandes "jump" et "duck" envoyées par le PC pour activer les bons servomoteurs (ex: pour lancer le jeu).
- [x] **Envoi Carte -> PC** : Envoyer le message "Jump" sur le moniteur série quand le dinosaure saute.
- [x] **Envoi Carte -> PC** : Envoyer le message "Duck" sur le moniteur série quand le dinosaure se baisse.

### A modif
- [x] modif ecran pr separer en deux
- [ ] clean code


### Bonus
- [ ] **Détection Jour/Nuit** : Implémenter un système capable de détecter quand le jeu passe du mode jour au mode nuit.
- [x] error handler
- [x] reset cmd fct
- [x] also return reset cmd on computer
- [ ] easter egg
- [x] 3 flash led on reset
- [x] 3 beep on reset