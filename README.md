# Crazy cylinder : simulation of the experiment
This file will contain explanation of the different part of the code, and maybe some link to the theory used for the numerical simulation.

# CFD 2D – Simulation de fluide incompressible

Ce projet implémente un **solveur CFD 2D** pour simuler un fluide incompressible autour d’un obstacle (cylindre) sur une grille rectangulaire. Le code est écrit en **C++** et utilise une architecture orientée objet pour la grille, les champs et le liquide.

---

## Structure du projet

- **Grille**  
  Gère la grille 2D, le pas de discrétisation, et l’obstacle circulaire via un masque de cellules solides.

- **Champ / Site**  
  Représente les champs scalaires ou vectoriels (vitesse, pression), avec accès facile par coordonnées `(x, y)` ou index 1D via `Site`.

- **Liquide**  
  Contient les vitesses, la pression, les champs intermédiaires et fournit des fonctions CFD pour calculer la divergence et le terme convectif.

- **Solveur**  
  Fonctions pour calculer le Laplacien, les gradients centraux et upwind, nécessaires pour résoudre les équations de Navier-Stokes.

---

## Fonctionnalités principales

- Simulation de fluide autour d’un cylindre (obstacle solide).  
- Accès aux champs modifiable ou en lecture seule via opérateurs `(x, y)` ou `Site`.  
- Calcul des gradients centraux et upwind pour stabiliser la convection.  
- Calcul de divergence pour la projection incompressible.  
- Base prête pour intégrer un solveur **SOR** pour la pression.  

## Authors
Jules Chamoy and Lofty Gauthier
