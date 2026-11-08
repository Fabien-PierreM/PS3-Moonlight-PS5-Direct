# PS3 Remote Play Relay — prototype 720p

Fork local de `Cruslan/PS3-Moonlight`, figé sur le commit `601ad9166359cb4032f13d33418e39a03b7523b8` (5 novembre 2026). Le paquet produit est une application PS3 distincte : **PS3 Remote Play Relay** (`PS5RL0001`).

## Limite de sécurité et de compatibilité

Ce dépôt **ne désactive pas** l'authentification Remote Play et ne contourne pas PSN, le compte PSN, l'appairage de la PS5 ou les clés d'enregistrement. Si « sans PSN » signifie *sans compte autorisé ni appairage officiel*, ce projet ne le permet pas.

Le prototype déplace la session Remote Play vers un **compagnon LAN** exécutant Chiaki-ng avec des éléments d'appairage obtenus de manière autorisée. La PS3 ne reçoit jamais ces éléments : elle ne reçoit qu'un flux H.264 Annex-B 720p progressif par UDP sur un réseau local de confiance. Cela évite que la PS3 ait besoin d'un client PSN, mais ne remplace pas les exigences d'autorisation de la PS5.

## Ce qui est réellement implémenté

- PKG PS3 séparé, titre `PS3 Remote Play Relay`, généré par la chaîne PSL1GHT/ps3dev incluse par le dépôt amont.
- Récepteur P3RP/1 LAN : UDP `9295`, réassemblage strict en ordre, accès H.264 Annex-B, plafond de 4 MiB par image.
- Profil PS3 fixe : `1280×720`, progressif, `60 fps`, H.264 ; décodage matériel `libvdec` et présentation RSX réutilisés du port PS3 Moonlight.
- Outil de test `tools/send_annexb_au.py` pour valider le chemin PS3 avec une unité d'accès H.264 locale.
- Bibliothèque de paquetisation `relay/` et adaptateur Chiaki-ng minimal : il force un profil Chiaki `720p / 60 / H.264` et attache le callback vidéo, sans gérer d'identifiants ni de PSN.

## Ce qui reste volontairement hors du prototype

- démarrage complet d'une session Chiaki-ng (le projet fournit l'adaptateur, pas un gestionnaire de secrets) ;
- audio, retour manette, haptique, FEC, réordonnancement UDP et chiffrement P3RP/1 ;
- validation sur PS3 physique : la compilation crée un PKG, mais le rendu réel exige une PS3 CFW/HEN et un relais H.264 compatible.

Consulter `docs/ARCHITECTURE.md`, `docs/PROTOCOL.md` et `docs/BUILD.md` avant installation.
