# Port direct de Chiaki-ng sur PS3 — cible corrigée

## But

Construire une application PS3 native qui établit elle-même une session **Chiaki-ng / Remote Play PS5 sur le LAN** et affiche la vidéo directement via le VDEC/RSX de la PS3.

```text
PS5 (Remote Play LAN autorisé)
          │
          │ protocole Remote Play : session, chiffrement, contrôle, vidéo
          ▼
PS3 : libchiaki porté PPU + intégration PSL1GHT
          │
          ├─ callback vidéo H.264 → src/video.c → cellVdec / RSX
          ├─ ioPad → ChiakiControllerState
          ├─ audio Opus → SPU / audio PS3 (phase ultérieure)
          └─ configuration locale chiffrée/privée sous USRDIR
```

Il n'y a **pas de PC au runtime**. Un PC reste seulement nécessaire pour compiler le PKG et, si besoin, transférer vers la PS3 les données d'inscription Remote Play obtenues légitimement.

## Ce que cela remplace

Le chemin `PS5 → PC Chiaki-ng → P3RP/1 → PS3` de la branche précédente est un prototype de relais, pas la cible finale. Cette branche `direct-chiaki-ps3` le conserve comme référence de rendu vidéo, mais ne doit pas exiger de PC quand le port direct sera terminé.

Moonlight ne peut pas être le client direct : il parle GameStream/Sunshine. Le moteur à porter est `libchiaki`.

## Surface libchiaki nécessaire

À intégrer pour une première session LAN PS5 sans interface graphique Chiaki :

- `session`, `ctrl`, `streamconnection`, `rpcrypt`, `takion`, `ecdh` ;
- récepteur/assembleur vidéo, FEC, bitstream et callback vidéo ;
- sockets, threads, timers, HTTP/curl et crypto mbedTLS ;
- enregistrement/import local, sans extraction ni contournement de clés ;
- mapping manette PS3 vers `ChiakiControllerState`.

À exclure explicitement de la première étape : GUI Qt, FFMPEG, découverte libevent, UPnP, Internet Remote Play/RUDP, micro, haptique et audio. La première démonstration fonctionnelle est **vidéo H.264 + commandes de base sur le LAN**.

## Inventaire PSL1GHT vérifié

La chaîne `Cruslan/ps3dev` de ce projet fournit déjà les en-têtes et bibliothèques nécessaires pour `curl`, `mbedTLS`, `json-c`, pthreads et sockets POSIX. Les portlibs absentes pour le chemin Chiaki complet sont `miniupnpc`, `libevent`, Jerasure et Opus. Elles ne doivent pas être ajoutées aveuglément :

- `miniupnpc`, `libevent`, RUDP et hole-punching sont hors périmètre LAN initial ;
- Nanopb est déjà fourni dans l'arbre Chiaki-ng ;
- Jerasure devra être porté/vendored pour la récupération FEC vidéo ;
- l'Opus existant de PS3-Moonlight peut être réemployé plus tard pour l'audio.

## Import de la console déjà enregistrée

La PS3 doit recevoir une configuration d'inscription **fournie volontairement par son propriétaire** : adresse LAN de la PS5, informations d'inscription nécessaires à une session autorisée et Account ID dans le format que `libchiaki` attend. Aucun secret ne doit être affiché dans les journaux, inscrit dans le dépôt ou copié dans le PKG.

Une inscription déjà présente sur un PC Chiaki-ng prouve que la PS5 est prête au Remote Play, mais ne rend pas automatiquement la PS3 inscrite : chaque client doit disposer de son propre état d'inscription valide, ou importer celui que son propriétaire a explicitement choisi de réutiliser.

## Jalons vérifiables

1. **Base PPU compilable — atteint** : wrapper PSL1GHT pour sockets, threads, temps, log et FEC ; aucune session réelle.
2. **Session LAN** — PS3 initialise `ChiakiSession` avec une configuration autorisée et atteint l'événement connecté.
3. **Vidéo directe** — callback `ChiakiVideoSampleCallback` pousse une image H.264 vers `ps3video_submit_annexb_access_unit()` ; l'image PS5 apparaît sur HDMI.
4. **Entrées** — la manette PS3 alimente `chiaki_session_set_controller_state()`.
5. **Audio** — Opus Chiaki relié au chemin SPU/audio existant.
6. **PKG final** — aucune dépendance PC, interface pour importer/configurer le profil local, essais matériels PS5/PS3.

## État réel à cette étape

- rendu VDEC/RSX 720p : compilé dans le PKG actuel ;
- primitives PPU `sock`, log, threads et temps de libchiaki : compilées dans `libchiaki-ps3-platform.a` ;
- FEC vidéo (GF-Complete + Jerasure) : compilée dans `libchiaki-ps3-fec.a` ;
- API `ChiakiSession` et callback vidéo : vérifiés dans Chiaki-ng ;
- port PPU complet de `libchiaki` : en cours ;
- session PS5 depuis PS3 : **non testée** ;
- un PKG direct fonctionnel : **pas encore produit**.

Ne pas installer le PKG actuel en s'attendant à une connexion PS5 directe : il reste un récepteur de relais jusqu'aux jalons 1–3.
