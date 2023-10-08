#pragma once

#include <unordered_map>

#include <dti/dti_types.h>

class Monster {
public:
    enum Id : u32;

    virtual ~Monster() = 0 {}
    virtual void* CreateUi(void*) = 0;
    virtual bool IsEnableInstance() = 0;
    virtual void CreateProperty(void*) = 0;
    virtual MtDTI<Monster>* GetDTI() = 0;

    [[nodiscard]] Id id() const { return get<Id>(0x12280); }
    [[nodiscard]] u32 variant() const { return get<u32>(0x12288); }
    [[nodiscard]] const char* name() const { return to_string(id()); }

    u32 custom_variant() const {
        const auto path = get<const char*>(0x5F28);

        u32 id, variant, n0, n1;
        if (sscanf_s(path, R"(em\em%03d\%02d\mod\em%03d_%02d)", &id, &variant, &n0, &n1) > 0) {
            return variant;
        }

        return 0;
    }

    MtVector3& pos() const { return get<MtVector3>(0x160); }
    MtVector3& size() const { return get<MtVector3>(0x180); }
    MtVector3& collision_pos() const { return get<MtVector3>(0xA50); }
    MtQuaternion& rot() const { return get<MtQuaternion>(0x170); }

    void teleport(const MtVector3& position) const { pos() = collision_pos() = position; }
    void resize(float factor) const { size() = { factor, factor, factor }; }

    float get_hp() const { return get<float*>(0x7670)[25]; }
    float get_max_hp() const { return get<float*>(0x7670)[24]; }

    float& speed() const { return get<float>(0x1D8A8); }
    float& despawn_time() const { return get<float>(0x1C3D4); }

    void set_frozen(bool frozen) const { ((void (*)(const void*, bool))0x1422456D0)(this, frozen); }

    void enrage() const { ((bool (*)(void*))0x1402a8da0)(&get<void*>(0x1BD08)); }
    void unenrage() const { ((bool (*)(void*))0x1402a9030)(&get<void*>(0x1BD08)); }

    float& animation_frame() const { return get<float>(animation_layer(), 0x10C); }
    float& max_animation_frame() const { return get<float>(animation_layer(), 0x114); }

    void spawn_effect(u32 group_id, u32 effect_id) const {
        const auto component = effect_component();
        const auto epv = get<void*>(component, 0x60);
        const auto effect = ((void*(*)(void*, u32, u32))0x14237fee0)(epv, group_id, effect_id);
        ((void*(*)(void*, u8, void*, bool))0x1412d4c60)(component, 0, effect, false);
    }
    void spawn_effect(void* epv, u32 group_id, u32 effect_id) const {
        const auto effect = ((void* (*)(void*, u32, u32))0x14237fee0)(epv, group_id, effect_id);
        ((void*(*)(void*, u8, void*, bool))0x1412d4c60)(effect_component(), 0, effect, false);
    }
    void spawn_shell(u32 index, const MtVector3& source, const MtVector3& dest) {
        float creation_params[73] = { 0 };
        set_shell_creation_params(creation_params, source, dest);
        ((void* (*)(void**, u32, u32, void*, void*, float*))0x141aba5f0)(&get<void*>(0x56E8), 0, index, this, this, creation_params);
    }
    void spawn_shell(void* shll, u32 index, const MtVector3& source, const MtVector3& dest) {
        float creation_params[73] = { 0 };
        set_shell_creation_params(creation_params, source, dest);
        ((void* (*)(void**, u32, u32, void*, void*, float*))0x141aba5f0)(&shll, 0, index, this, this, creation_params);
    }

    [[nodiscard]] static const char* to_string(Id id) {
        const auto found = Names.find(id);
        return found != Names.end() ? found->second : "Unknown";
    }

    template <class T> T& get(size_t offset) const {
        return *(T*)((size_t)this + offset);
    }

    enum Id : u32 {
        Anjanath = 0x00,
        Rathalos = 0x01,
        Aptonoth = 0x02,
        Jagras = 0x03,
        ZorahMagdaros = 0x04,
        Mosswine = 0x05,
        Gajau = 0x06,
        GreatJagras = 0x07,
        KestodonM = 0x08,
        Rathian = 0x09,
        PinkRathian = 0x0A,
        AzureRathalos = 0x0B,
        Diablos = 0x0C,
        BlackDiablos = 0x0D,
        Kirin = 0x0E,
        Behemoth = 0x0F,
        KushalaDaora = 0x10,
        Lunastra = 0x11,
        Teostra = 0x12,
        Lavasioth = 0x13,
        Deviljho = 0x14,
        Barroth = 0x15,
        Uragaan = 0x16,
        Leshen = 0x17,
        Pukei = 0x18,
        Nergigante = 0x19,
        XenoJiiva = 0x1A,
        KuluYaKu = 0x1B,
        TzitziYaKu = 0x1C,
        Jyuratodus = 0x1D,
        TobiKadachi = 0x1E,
        Paolumu = 0x1F,
        Legiana = 0x20,
        GreatGirros = 0x21,
        Odogaron = 0x22,
        Radobaan = 0x23,
        VaalHazak = 0x24,
        Dodogama = 0x25,
        KulveTaroth = 0x26,
        Bazelgeuse = 0x27,
        Apceros = 0x28,
        KelbiM = 0x29,
        KelbiF = 0x2A,
        Hornetaur = 0x2B,
        Vespoid = 0x2C,
        Mernos = 0x2D,
        KestodonF = 0x2E,
        Raphinos = 0x2F,
        Shamos = 0x30,
        Barnos = 0x31,
        Girros = 0x32,
        AncientLeshen = 0x33,
        Gastodon = 0x34,
        Noios = 0x35,
        Magmacore = 0x36,
        Magmacore2 = 0x37,
        Gajalaka = 0x38,
        SmallBarrel = 0x39, // training objects
        LargeBarrel = 0x3A,
        TrainingPole = 0x3B,
        TrainingWagon = 0x3C,
        Tigrex = 0x3D,
        Nargacuga = 0x3E,
        Barioth = 0x3F,
        SavageDeviljho = 0x40,
        Brachydios = 0x41,
        Glavenus = 0x42,
        AcidicGlavenus = 0x43,
        FulgurAnjanath = 0x44,
        CoralPukei = 0x45,
        RuinerNergigante = 0x46,
        ViperTobi = 0x47,
        NightshadePaolumu = 0x48,
        ShriekingLegiana = 0x49,
        EbonyOdogaron = 0x4A,
        BlackveilVaal = 0x4B,
        SeethingBazelgeuse = 0x4C,
        Beotodus = 0x4D,
        Banbaro = 0x4E,
        Velkhana = 0x4F,
        Namielle = 0x50,
        Shara = 0x51,
        Popo = 0x52,
        Anteka = 0x53,
        Wulg = 0x54,
        Cortos = 0x55,
        Boaboa = 0x56,
        Alatreon = 0x57,
        GoldRathian = 0x58,
        SilverRathalos = 0x59,
        YianGaruga = 0x5A,
        Rajang = 0x5B,
        FuriousRajang = 0x5C,
        BruteTigrex = 0x5D,
        Zinogre = 0x5E,
        StygianZinogre = 0x5F,
        RagingBrachy = 0x60,
        SafiJiiva = 0x61,
        Unavaliable = 0x62,
        ScarredYianGaruga = 0x63,
        FrostfangBarioth = 0x64,
        Fatalis = 0x65
    };

private:
    template <class T> static T& get(void* base, size_t offset) {
        return *(T*)((size_t)base + offset);
    }

    void* animation_layer() const { return get<void*>(0x468); }
    void* effect_component() const { return get<void*>(0xA10); }
    void set_shell_creation_params(float* params, const MtVector3& source, const MtVector3& dest) const {
        params[0] = source.x;
        params[1] = source.y;
        params[2] = source.z;
        params[3] = 0.0f;
        get<bool>(params, 0x10) = true;

        params[16] = dest.x;
        params[17] = dest.y;
        params[18] = dest.z;
        params[19] = 0.0f;
        get<bool>(params, 0x50) = true;

        get<u32>(params, 0xA0) = 0x12;
        get<s64>(params, 0xA4) = -1;
    }

    static inline const std::unordered_map<Id, const char*> Names = {
        {Id::Anjanath, "Anjanath"},
        {Id::Rathalos, "Rathalos"},
        {Id::Aptonoth, "Aptonoth"},
        {Id::Jagras, "Jagras"},
        {Id::ZorahMagdaros, "Zorah Magdaros"},
        {Id::Mosswine, "Mosswine"},
        {Id::Gajau, "Gajau"},
        {Id::GreatJagras, "Great Jagras"},
        {Id::KestodonM, "Kestodon M"},
        {Id::Rathian, "Rathian"},
        {Id::PinkRathian, "Pink Rathian"},
        {Id::AzureRathalos, "Azure Rathalos"},
        {Id::Diablos, "Diablos"},
        {Id::BlackDiablos, "Black Diablos"},
        {Id::Kirin, "Kirin"},
        {Id::Behemoth, "Behemoth"},
        {Id::KushalaDaora, "Kushala Daora"},
        {Id::Lunastra, "Lunastra"},
        {Id::Teostra, "Teostra"},
        {Id::Lavasioth, "Lavasioth"},
        {Id::Deviljho, "Deviljho"},
        {Id::Barroth, "Barroth"},
        {Id::Uragaan, "Uragaan"},
        {Id::Leshen, "Leshen"},
        {Id::Pukei, "Pukei"},
        {Id::Nergigante, "Nergigante"},
        {Id::XenoJiiva, "Xeno'Jiiva"},
        {Id::KuluYaKu, "Kulu-Ya-Ku"},
        {Id::TzitziYaKu, "Tzitzi-Ya-Ku"},
        {Id::Jyuratodus, "Jyuratodus"},
        {Id::TobiKadachi, "Tobi Kadachi"},
        {Id::Paolumu, "Paolumu"},
        {Id::Legiana, "Legiana"},
        {Id::GreatGirros, "Great Girros"},
        {Id::Odogaron, "Odogaron"},
        {Id::Radobaan, "Radobaan"},
        {Id::VaalHazak, "Vaal Hazak"},
        {Id::Dodogama, "Dodogama"},
        {Id::KulveTaroth, "Kulve Taroth"},
        {Id::Bazelgeuse, "Bazelgeuse"},
        {Id::Apceros, "Apceros"},
        {Id::KelbiM, "Kelbi"},
        {Id::KelbiF, "Kelbi2"},
        {Id::Hornetaur, "Hornetaur"},
        {Id::Vespoid, "Vespoid"},
        {Id::Mernos, "Mernos"},
        {Id::KestodonF, "Kestodon F"},
        {Id::Raphinos, "Raphinos"},
        {Id::Shamos, "Shamos"},
        {Id::Barnos, "Barnos"},
        {Id::Girros, "Girros"},
        {Id::AncientLeshen, "Ancient Leshen"},
        {Id::Gastodon, "Gastodons"},
        {Id::Noios, "Noios"},
        {Id::Gajalaka, "Gajalaka"},
        {Id::Tigrex, "Tigrex"},
        {Id::Nargacuga, "Nargacuga"},
        {Id::Barioth, "Barioth"},
        {Id::SavageDeviljho, "Savage Deviljho"},
        {Id::Brachydios, "Brachydios"},
        {Id::Glavenus, "Glavenus"},
        {Id::AcidicGlavenus, "Acidic Glavenus"},
        {Id::FulgurAnjanath, "Fulgur Anjanath"},
        {Id::CoralPukei, "Coral Pukei"},
        {Id::RuinerNergigante, "Ruiner Nergigante"},
        {Id::ViperTobi, "Viper Tobi Kadachi"},
        {Id::NightshadePaolumu, "Nightshade Paolumu"},
        {Id::ShriekingLegiana, "Shrieking Legiana"},
        {Id::EbonyOdogaron, "Ebony Odogaron"},
        {Id::BlackveilVaal, "Blackveil Vaal Hazak"},
        {Id::SeethingBazelgeuse, "Seething Bazelgeuse"},
        {Id::Beotodus, "Beotodus"},
        {Id::Banbaro, "Banbaro"},
        {Id::Velkhana, "Velkhana"},
        {Id::Namielle, "Namielle"},
        {Id::Shara, "Shara Ishvalda"},
        {Id::Popo, "Popo"},
        {Id::Anteka, "Anteka"},
        {Id::Wulg, "Wulg"},
        {Id::Cortos, "Cortos"},
        {Id::Boaboa, "Boaboa"},
        {Id::Alatreon, "Alatreon"},
        {Id::GoldRathian, "Gold Rathian"},
        {Id::SilverRathalos, "Silver Rathalos"},
        {Id::YianGaruga, "Yian Garuga"},
        {Id::Rajang, "Rajang"},
        {Id::FuriousRajang, "Furious Rajang"},
        {Id::BruteTigrex, "Brute Tigrex"},
        {Id::Zinogre, "Zinogre"},
        {Id::StygianZinogre, "Stygian Zinogre"},
        {Id::RagingBrachy, "Raging Brachydios"},
        {Id::SafiJiiva, "Safi'Jiiva"},
        {Id::Unavaliable, "Unavaliable"},
        {Id::ScarredYianGaruga, "Scarred Yian Garuga"},
        {Id::FrostfangBarioth, "Frostfang Barioth"},
        {Id::Fatalis, "Fatalis"},
        {Id::SmallBarrel, "Small Barrel"},
        {Id::LargeBarrel, "Large Barrel"},
        {Id::TrainingPole, "Training Pole"},
        {Id::Magmacore, "Magmacore"},
        {Id::Magmacore2, "Magmacore2"}
    };
};