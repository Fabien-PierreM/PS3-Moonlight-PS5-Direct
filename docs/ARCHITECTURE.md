# Architecture : PS5 → Chiaki-ng → PS3

```text
PS5 Remote Play (LAN)
        │  session autorisée / matériel d'appairage conservé côté compagnon
        ▼
Compagnon LAN : Chiaki-ng/libchiaki
  - `ChiakiConnectInfo.video_profile`: 720p, 60 fps, H.264
  - `chiaki_session_set_video_sample_cb()`
  - adaptateur `relay/src/chiaki_adapter.c`
        │  P3RP/1 : unités d'accès H.264 Annex-B fragmentées en UDP
        │  contrôle best-effort UDP/9296 ; vidéo UDP/9295
        ▼
PS3 Remote Play Relay (`PS5RL0001`)
  - `src/ps5_relay.c`: réassemblage d'une image, plafond 4 MiB
  - `src/video.c`: wrapper Annex-B → `cellVdec` / RSX existant
  - 1280×720p60 H.264 fixe
        ▼
HDMI / écran PS3
```

## Décisions

1. **Pas de port direct complet de libchiaki sur PS3.** Le `lib/CMakeLists.txt` de Chiaki-ng rassemble notamment curl, json-c, miniupnpc, libevent, crypto, nanopb et jerasure. Le port direct ajouterait une importante surface de portage et de sécurité à PSL1GHT sans résoudre l'autorisation Remote Play.
2. **H.264 uniquement.** Le code PS3 existant initialise `VDEC_CODEC_TYPE_H264` et sait déjà convertir AVCC/Annex-B avant soumission à VDEC. Le relais force donc `CHIAKI_CODEC_H264`; aucune voie H.265/HDR n'est annoncée.
3. **Le relais garde les secrets.** Les clés, Account ID et l'état d'appairage sont exclus du format P3RP/1, de la configuration PS3 et du PKG.
4. **Protocole de données séparé.** P3RP/1 n'est ni le protocole propriétaire Remote Play ni une réimplémentation de celui-ci. C'est un transport local borné d'unités H.264 déjà déchiffrées par un client autorisé.

## Arborescence ajoutée

```text
shared/ps3_relay_protocol.h     format P3RP/1 partagé
src/ps5_relay.[ch]              récepteur PS3 + contrôle best-effort
src/video.[ch]                  wrappers publics vers le décodeur VDEC existant
relay/include/packetizer.h      paquetisation POSIX générique
relay/src/packetizer.c          expéditeur P3RP/1 testé indépendamment
relay/include/chiaki_adapter.h  point d'intégration libchiaki
relay/src/chiaki_adapter.c      profil 720p60/H.264 + callback vidéo
tools/send_annexb_au.py         test de données sans PS5 ni PSN
docs/PROTOCOL.md                contrat binaire réseau
docs/BUILD.md                   compilation et validation reproductibles
```

## Dépôts vérifiés et versions gelées

| Composant | Dépôt | Révision utilisée/analysée | Rôle |
|---|---|---:|---|
| Base PS3 | `https://github.com/Cruslan/PS3-Moonlight` | `601ad9166359cb4032f13d33418e39a03b7523b8` | UI, PKG, `libvdec`, RSX, Opus, PSL1GHT |
| SDK PS3 précompilé | `https://github.com/Cruslan/ps3dev` | `71a42f60744c56638941935a4283b02a4fc6a633` | archive Linux/macOS demandée par `make prepare` |
| Relais Remote Play | `https://github.com/chiaki-ng/chiaki-ng` | `43728af2e639fecb8a7af1cd68c29820a40de973` | API `ChiakiSession`, profil H.264 et callback vidéo |
| Toolchain de référence | `https://github.com/ps3dev/ps3dev` | non vendored | contexte PSL1GHT/portlibs |

Les commits ci-dessus ont été vérifiés le 7 novembre 2026. Les dépendances du compagnon restent sous leurs licences propres : vérifier notamment la compatibilité de la licence de Chiaki-ng avant toute distribution combinée.
