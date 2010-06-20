/*
	ISO image file search

	Copyright (C) 2010	Patrice Mandin

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "md5.h"
#include "background_tim.h"
#include "param.h"

/*--- Defines ---*/

#define FILE_TIM_4	0
#define FILE_TIM_8	1
#define FILE_TIM_16	2
#define FILE_EMD	3
#define FILE_DO3	4

#define DATA_LENGTH 2048
#define MAX_FILE_SIZE (512<<10)

/*--- Types ---*/

typedef struct {
	const char *value;	/* MD5 value */
	const char *filename;	/* Filename */
	int found;
} md5_check_t;

/*--- Constants ---*/

md5_check_t md5_checks[]={
	{"3199387aa01f9b4483859d7bdff1ba99","data/etc/capcom.tim", 0},
	{"e66a2dd333f61ba00359b070c5f55e47","data/etc/continue.tim", 0},
	{"ee67bc522607a3c707d0be2b6215a76d","data/etc/eidos.tim", 0},
	{"9f2f16eeb762d31cd857d7e4b5792f56","data/etc/filei.tim", 0},
	{"9891a5bf8bdf355a450bbd19cb0c0e2e","data/etc/radar.tim", 0},

	{"1de0325d42dc7a70d80bbf43f3b4abc9","data_a/pld/pl000.tim", 0},
	{"53d47acf108f767564929a2507cc5616","data_a/pld/pl006.tim", 0},

	{"7ea884f111d98e29316e494bc4429936","data_af/etc2/filegf03.tim", 0},
	{"e633271dc934a5656cea1a89d47e8cda","data_af/etc2/filegf04.tim", 0},
	{"7f68a6076bda5be42804490fc524d4a6","data_af/etc2/filegf05.tim", 0},
	{"b36b78b4d08a9fceb3e154784eeea20f","data_af/etc2/filegf06.tim", 0},
	{"d73711137ae535f947e8abb35b9e5555","data_af/etc2/filegf07.tim", 0},
	{"78ea6982929d3798770ec97cece8efa2","data_af/etc2/filegf08.tim", 0},
	{"77e05992512b975e468a6464eefbcca3","data_af/etc2/filegf09.tim", 0},
	{"ac97d763a9b5d62a247ac3106085d733","data_af/etc2/filegf10.tim", 0},
	{"007aa157b73f3ede33d2c43925fa3905","data_af/etc2/titlef03.tim", 0},

	{"0187053a56e14d76f5358613b76440da","data_au/etc2/filegu03.tim", 0},
	{"a8298626dc79f0fe1371f9fa0f7a5c3f","data_au/etc2/filegu04.tim", 0},
	{"52e3f28b7ade94fbf6c734b06c65b088","data_au/etc2/filegu05.tim", 0},
	{"583cadf1d7f72fb8c0620730008a1132","data_au/etc2/filegu06.tim", 0},
	{"95361274b394b496d5d8d7f83f9a4ffa","data_au/etc2/filegu07.tim", 0},
	{"4bbf70625d883a076525311fdf27985d","data_au/etc2/filegu08.tim", 0},
	{"c9b4f780e38847c0566787f6fada6329","data_au/etc2/filegu09.tim", 0},
	{"f987f1d0ec9ed8b9da8e64c2cca0c130","data_au/etc2/filegu10.tim", 0},
	{"983a0b93e5fd771ed379e25b1c8de10d","data_au/etc2/titleu03.tim", 0},

	{"9dbfc72e924087799b692382d24acdc3","data_ae/etc2/filege03.tim", 0},
	{"a8298626dc79f0fe1371f9fa0f7a5c3f","data_ae/etc2/filege04.tim", 0},
	{"124c42bf6f479338a752397153e8ecc7","data_ae/etc2/filege05.tim", 0},
	{"c0f4bc5a5758953761ce40f4a77aad51","data_ae/etc2/filege06.tim", 0},
	{"3deda931b27840a69a822c209166a160","data_ae/etc2/filege07.tim", 0},
	{"4ff1465d9bfaa33d79b7d528e600a455","data_ae/etc2/filege08.tim", 0},
	{"acec10d5c4c4b1a5b67803afc3eb9951","data_ae/etc2/filege09.tim", 0},
	{"7495db72e68014becf3a01de8ff59d5a","data_ae/etc2/filege10.tim", 0},
	{"983a0b93e5fd771ed379e25b1c8de10d","data_ae/etc2/titlee03.tim", 0},

	{"060aee019f3f4af95682648d0d3d4a48","data_f/etc2/core00f.tim", 0},
	{"f6c102ae8c82d14ad4c9e28ea85df905","data_f/etc2/died00f.tim", 0},
	{"0682b60cb12ba23fe85fbc59be76901e","data_f/etc2/fileif.tim", 0},
	{"ea4a596937e502b991b1c1d984ea0c08","data_f/etc2/jill_bgf.tim", 0},
	{"da5ffe2f28f6bb2f369a900208c0b51e","data_f/etc2/jill_obf.tim", 0},
	{"b4abb0c6ca41115e7009cc4b3b9a05c1","data_f/etc2/res0_bgf.tim", 0},
	{"f3d53210b23328151dad7ed2725e0f92","data_f/etc2/res0_obf.tim", 0},
	{"c5549bc68a0b0717e5ea14505a81ca73","data_f/etc2/res3_bgf.tim", 0},
	{"e798f1d09e589e895c87848c4c5615bc","data_f/etc2/res4_bgf.tim", 0},
	{"d7e79a107a0be7676f340ab6480b9146","data_f/etc2/res5_bgf.tim", 0},
	{"d4c5520737e6a43633e64044252eb6d4","data_f/etc2/sele_bgf.tim", 0},
	{"3ddabd8f78e7ce68a78555d6374884d7","data_f/etc2/sele_obf.tim", 0},
	{"2f25df1830a829b7633ac03a06158c01","data_f/etc2/stmain0f.tim", 0},
	{"3b4fb7d190186ea6b370034228e5a2f9","data_f/etc2/stmain1f.tim", 0},
	{"ae858af6be0e745a544a0ddb62b37886","data_f/etc2/stmain2f.tim", 0},
	{"6ef1ddba2db5639d1c8ef19ab14127c6","data_f/etc2/stmain3f.tim", 0},
	{"6cf2a4173e18c999c06e4afbd1238da2","data_f/etc2/stmojif.tim", 0},
	{"0c4d429e7ae6cd14fa3a4a70f47e8bee","data_f/etc2/texf.tim", 0},
	{"a3e94b4fd87d5e1177eb79a1d51df32f","data_f/etc2/warnf.tim", 0},

	{"060aee019f3f4af95682648d0d3d4a48","data_u/etc2/core00u.tim", 0},
	{"c4e05c9e7bea68cff76ce917f64729b3","data_u/etc2/died00u.tim", 0},
	{"9f2f16eeb762d31cd857d7e4b5792f56","data_u/etc2/fileiu.tim", 0},
	{"ea4a596937e502b991b1c1d984ea0c08","data_u/etc2/jill_bgu.tim", 0},
	{"3e761cf7911c861bdf0d5222920c0ef3","data_u/etc2/jill_obu.tim", 0},
	{"b4abb0c6ca41115e7009cc4b3b9a05c1","data_u/etc2/res0_bgu.tim", 0},
	{"9201a0e1e8eb60ede2a6129a91f078d2","data_u/etc2/res0_obu.tim", 0},
	{"cac4563872acd7a385c72cc236fd8660","data_u/etc2/res2_bgu.tim", 0},
	{"c5549bc68a0b0717e5ea14505a81ca73","data_u/etc2/res3_bgu.tim", 0},
	{"f14c05a19be463198f2728237aa30971","data_u/etc2/res4_bgu.tim", 0},
	{"d7e79a107a0be7676f340ab6480b9146","data_u/etc2/res5_bgu.tim", 0},
	{"d4c5520737e6a43633e64044252eb6d4","data_u/etc2/sele_bgu.tim", 0},
	{"cc80746fbee129bf18501b5be2d504db","data_u/etc2/sele_obu.tim", 0},
	{"80821521d23a97dfcc930454de6bc67a","data_u/etc2/stmain0u.tim", 0},
	{"eb6a9933439aec065738482fffa188cf","data_u/etc2/stmain1u.tim", 0},
	{"1541018e0c6723ee8b8f0497234d2616","data_u/etc2/stmain2u.tim", 0},
	{"168d4a01eaf0fe54dfb8a7331d957575","data_u/etc2/stmain3u.tim", 0},
	{"1d2efac68cc5fc1c1d86aaa0126c43d1","data_u/etc2/stmojiu.tim", 0},
	{"d6f2241418311622eca1258f14a2a1c1","data_u/etc2/texu.tim", 0},
	{"eee1894d64e256144ff783327fd10e09","data_u/etc2/warnu.tim", 0},

	{"060aee019f3f4af95682648d0d3d4a48","data_e/etc2/core00e.tim", 0},
	{"c4e05c9e7bea68cff76ce917f64729b3","data_e/etc2/died00e.tim", 0},
	{"9f2f16eeb762d31cd857d7e4b5792f56","data_e/etc2/fileie.tim", 0},
	{"ea4a596937e502b991b1c1d984ea0c08","data_e/etc2/jill_bge.tim", 0},
	{"3e761cf7911c861bdf0d5222920c0ef3","data_e/etc2/jill_obe.tim", 0},
	{"b4abb0c6ca41115e7009cc4b3b9a05c1","data_e/etc2/res0_bge.tim", 0},
	{"9201a0e1e8eb60ede2a6129a91f078d2","data_e/etc2/res0_obe.tim", 0},
	{"c5549bc68a0b0717e5ea14505a81ca73","data_e/etc2/res3_bge.tim", 0},
	{"f14c05a19be463198f2728237aa30971","data_e/etc2/res4_bge.tim", 0},
	{"d7e79a107a0be7676f340ab6480b9146","data_e/etc2/res5_bge.tim", 0},
	{"d4c5520737e6a43633e64044252eb6d4","data_e/etc2/sele_bge.tim", 0},
	{"8a07218711bd2da04a9eef84ee55fb9b","data_e/etc2/sele_obe.tim", 0},
	{"80821521d23a97dfcc930454de6bc67a","data_e/etc2/stmain0e.tim", 0},
	{"eb6a9933439aec065738482fffa188cf","data_e/etc2/stmain1e.tim", 0},
	{"1541018e0c6723ee8b8f0497234d2616","data_e/etc2/stmain2e.tim", 0},
	{"168d4a01eaf0fe54dfb8a7331d957575","data_e/etc2/stmain3e.tim", 0},
	{"1d2efac68cc5fc1c1d86aaa0126c43d1","data_e/etc2/stmojie.tim", 0},
	{"d6f2241418311622eca1258f14a2a1c1","data_e/etc2/texe.tim", 0},
	{"eee1894d64e256144ff783327fd10e09","data_e/etc2/warne.tim", 0},

	{"9bcc78189acac849f6cda0986ce084d9","room/emd/em10.emd", 0},
	{"02613fdd2a9402284c7c906283d80973","room/emd/em10.tim", 0},
	{"b3172c3aa228533c637e328d84a4a0e7","room/emd/em11.emd", 0},
	{"0c29e033124028068ca56da61a19c4a7","room/emd/em11.tim", 0},
	{"c16bbc8652accffca210b3047dcaf810","room/emd/em12.emd", 0},
	{"7e04298f5a5adecb14b9d60c9b2d163e","room/emd/em12.tim", 0},
	{"523090faa3f1e3bc3842fda8c13a20fb","room/emd/em13.emd", 0},
	{"db00767cce785ef6da1c8e5755fbfe2f","room/emd/em13.tim", 0},	/* = room/emd08/em13.tim */
	{"6e8153cd2db6405aeb2671471b22bb35","room/emd/em14.emd", 0},
	{"1225b45f80f7a991830d39ec23fd3e20","room/emd/em14.tim", 0},
	{"318b4f532fd391a484ea28efaefa1c70","room/emd/em15.emd", 0},
	{"002bf41b7c8dab37cc177aba973b910f","room/emd/em15.tim", 0},
	{"3401c8a6295f9a965095377bef78c093","room/emd/em16.emd", 0},
	{"098a614f991bdaefdb706fac38900a89","room/emd/em16.tim", 0},
	{"4b46fb238b5078434093950b7d3539aa","room/emd/em17.emd", 0},
	{"a4acbb1a32c3301b9a722cf7f4d50f4a","room/emd/em17.tim", 0},
	{"ae9b192d93a9af6b9270cdd279d388c1","room/emd/em18.emd", 0},
	{"bd64827a53ac83376a37fb8540e42239","room/emd/em18.tim", 0},
	{"f62b85aa2410aba623052652978e9a4b","room/emd/em19.emd", 0},
	{"d04ef240fd3157785bd3f46feabc0de6","room/emd/em19.tim", 0},
	{"f8a39d03ed7f568b78cf5232a351f1d4","room/emd/em1a.emd", 0},
	{"47f3d7b46cc1e57d637ac328e17d7a26","room/emd/em1a.tim", 0},
	{"fa7e9e13ae269c1507c5abae56368f18","room/emd/em1b.emd", 0},
	{"d0188eb8a5e34bdd0f78a899e1b0bcc5","room/emd/em1b.tim", 0},
	{"f3bb3369dc66a3b3f44643a2fff9dec6","room/emd/em1c.emd", 0},
	{"8f37dfd58aab16ea2bb67d68529d5a21","room/emd/em1c.tim", 0},
	{"b01ebf1df508fb1d56910559c7251a11","room/emd/em1d.emd", 0},
	{"907c31e1adbe9807c256a4842cfb9a15","room/emd/em1d.tim", 0},
	{"3401c8a6295f9a965095377bef78c093","room/emd/em1e.emd", 0},
	{"098a614f991bdaefdb706fac38900a89","room/emd/em1e.tim", 0},
	{"c94f3ceb8c613d4f9ef4e3838fd68b26","room/emd/em1f.emd", 0},
	{"fcb5c49f6ebba4330c0376ed65b751f9","room/emd/em1f.tim", 0},
	{"0e1ec3e2b13de764b9b9cc9850eeb7e1","room/emd/em20.emd", 0},
	{"b8ad398255ae64ebe1eebb77ce0984a9","room/emd/em20.tim", 0},
	{"84446e90460e558418d500902f99bd0a","room/emd/em21.emd", 0},
	{"413c9653df474b05f18f778a6cf5a1a2","room/emd/em21.tim", 0},
	{"ad896f4aaefa49e3186ef388f3affdd8","room/emd/em22.emd", 0},
	{"aeaf4bac057e0b19b23e85a707bf4631","room/emd/em22.tim", 0},	/* = room/emd08/em22.tim */
	{"05c86b22e071ac58190219bba7af072c","room/emd/em23.emd", 0},
	{"77d6dc3597fedd07dee5c2d4fddc3ab6","room/emd/em23.tim", 0},
	{"6f168094b6c9838546398d9c21d0a055","room/emd/em24.emd", 0},
	{"6a5aae2906ea61bd978513811e7dc050","room/emd/em24.tim", 0},	/* = room/emd08/em24.tim */
	{"97d0268b25e62a84d1dfe51498e04a4d","room/emd/em25.emd", 0},
	{"33d63344a232dc5292c5e9d3188a293b","room/emd/em25.tim", 0},
	{"84dde72548a5b980242bd268271d0f3c","room/emd/em26.emd", 0},
	{"09a18a39781045f65c527c13ce6dbf3b","room/emd/em26.tim", 0},
	{"9311c8e236bc04939c2e4607a5b105b6","room/emd/em27.emd", 0},
	{"7514ff8bbcbaca75b2b87c2d3c61c343","room/emd/em27.tim", 0},
	{"6bb3c76fa7e079bfa59095bfedde0dc7","room/emd/em28.emd", 0},
	{"1dde25c43a8027bae6e3c221b88c909f","room/emd/em28.tim", 0},
	{"942cd7da477c36402c1d95f8badefa28","room/emd/em2c.emd", 0},
	{"9fd98d162d710fd39e1a2d19ef84b25f","room/emd/em2c.tim", 0},
	{"9aa73cd765be331e3b33e5325b767988","room/emd/em2d.emd", 0},
	{"c965e1faa886bde5f873a4d92da0b35b","room/emd/em2d.tim", 0},
	{"f1bb239b039b546ff02fdb656a133f8a","room/emd/em2e.emd", 0},
	{"4bbe791db00d84a618abce4d23d66d99","room/emd/em2e.tim", 0},	/* = room/emd/em55.tim */
	{"ee2c79a51839e69a9677640f4d179f60","room/emd/em2f.emd", 0},
	{"b414d04bdb0fd4c4fb20927a1c85f4b8","room/emd/em2f.tim", 0},
	{"b3f81d8b8e638bb6bf7612f10b33d868","room/emd/em30.emd", 0},
	{"4cf5acb565dc57c9456f1961f6c3e284","room/emd/em30.tim", 0},
	{"ef3a11eae0d61dbfa7e2e620b698b0ca","room/emd/em32.emd", 0},
	{"a15eb28823b1463954861ba1100ca2b7","room/emd/em32.tim", 0},
	{"d26111f8fc18044000f655da29a5746b","room/emd/em33.emd", 0},
	{"a3c4eccccca8de15f006e7f6d13a6b7e","room/emd/em33.tim", 0},
	{"d820f094e6d85cc822dc1af3de626967","room/emd/em34.emd", 0},
	{"40c92c76cf55988fc8aaf6903fb74d6e","room/emd/em34.tim", 0},
	{"9f673280ee9bd2647eaec6c3512ba53b","room/emd/em35.emd", 0},
	{"fef483ea0d083f025528f572ced803b8","room/emd/em35.tim", 0},
	{"be080f67d85e8af8de3968f8cdccb502","room/emd/em36.emd", 0},
	{"f381d1228e1f293b4aba8815c77d0319","room/emd/em36.tim", 0},
	{"cb2caf355f68ba54faac06d8824db816","room/emd/em37.emd", 0},
	{"0166fe0b59d0ccd56ff6370a20d5cbe2","room/emd/em37.tim", 0},
	{"2d59fb9e12fc0bfd5f939e94dc4a215e","room/emd/em38.emd", 0},
	{"3d97aa2a862a825682faf8326185d151","room/emd/em38.tim", 0},
	{"13a0166a6bf6f57fdf5c0ae106aa7ca8","room/emd/em39.emd", 0},
	{"35203d7fc448e4a7b5398a9c52f9c840","room/emd/em39.tim", 0},
	{"1c6e07531be585a2cf09c2e27bca859c","room/emd/em3a.emd", 0},
	{"9043a2e9d4921f44d172adc219a2e1d7","room/emd/em3a.tim", 0},
	{"3bc3dac621d67b678141a3f606c303e2","room/emd/em3b.emd", 0},
	{"0166fe0b59d0ccd56ff6370a20d5cbe2","room/emd/em3b.tim", 0},
	{"85499323e9c8bf4887aeb1ba8bd49a50","room/emd/em3e.emd", 0},
	{"4a238804882f4fcacb9b20f13d117540","room/emd/em3e.tim", 0},
	{"f5efe027786879d0f4f24a438fa21edc","room/emd/em3f.emd", 0},
	{"4c32ccd33e33ea4bd640d6740b98f975","room/emd/em3f.tim", 0},
	{"7c3b495f2b1f9e1f4c23bc97b4e43f74","room/emd/em40.emd", 0},
	{"4edf797ab6cf75f49806499282b32ced","room/emd/em40.tim", 0},
	{"fb3b79cd7fc65baf6327c3574b8c551b","room/emd/em50.emd", 0},
	{"8a6b23c802dda1934922d00d5acfe3e8","room/emd/em50.tim", 0},
	{"f8dc87952885e36a41663189f3feaf56","room/emd/em51.emd", 0},
	{"bf68961bdd817cf8e827631d23127cef","room/emd/em51.tim", 0},
	{"10482a5caf2090c12d0481819bad5c4f","room/emd/em52.emd", 0},
	{"8ae1eb4e8c9a4b71dba96899da284397","room/emd/em52.tim", 0},
	{"e666720fd6d71c7acb9ab94bf12e43e0","room/emd/em53.emd", 0},
	{"58f6405de070393b1a1978fb2add9832","room/emd/em53.tim", 0},
	{"b756093ae4ecf24371ae578402cc4bd2","room/emd/em54.emd", 0},
	{"9cbfad29fbdb8b9c9c455840eab2d8bb","room/emd/em54.tim", 0},
	{"5ecff12c4ec3f4a3f7f5722c66b9755b","room/emd/em55.emd", 0},
	{"4bbe791db00d84a618abce4d23d66d99","room/emd/em55.tim", 0},	/* = room/emd/em2e.tim */
	{"152145fbf233f55128af760a289cf5b0","room/emd/em56.emd", 0},
	{"983c220257e26c614616d4892809eb79","room/emd/em56.tim", 0},
	{"fa75db12f62d6f89af75216e45c11758","room/emd/em57.emd", 0},
	{"b414d04bdb0fd4c4fb20927a1c85f4b8","room/emd/em57.tim", 0},
	{"95b9255cd0d01b927a750424a7580f3c","room/emd/em58.emd", 0},
	{"6589fe16175c3695c127baf1a5f7f571","room/emd/em58.tim", 0},
	{"a6ac51db0209ace9d332d9c22157104d","room/emd/em59.emd", 0},
	{"696356ddcf30d8245c915f6deada485d","room/emd/em59.tim", 0},
	{"c05d9862d6d259d36a2cfdf9b8c634cd","room/emd/em5a.emd", 0},
	{"db10b5519defebb6d388fa219d0985f3","room/emd/em5a.tim", 0},
	{"c07ca5c8773e7ef30a6b197d6f977729","room/emd/em5b.emd", 0},
	{"0ce4cd9daa73d3afb71e107aba1520d0","room/emd/em5b.tim", 0},
	{"80e51a1b7278e1578a4e1234c44e91df","room/emd/em5c.emd", 0},
	{"3d3e73855a963156f24b5f9049dd7d23","room/emd/em5c.tim", 0},
	{"f74bdc7cb7c1c08cd2012024affa321f","room/emd/em5d.emd", 0},
	{"dc777650bacc4d6855f223222511d3f2","room/emd/em5d.tim", 0},
	{"6a312fd033ebeaa92605f35f2cdebf39","room/emd/em5e.emd", 0},
	{"e02b686deeb531f82990e2105f7c9b75","room/emd/em5e.tim", 0},
	{"5584d19d51d1dd6321929743f9371c11","room/emd/em5f.emd", 0},
	{"5c7dbae23df4467d97c8ff2f3097bdb5","room/emd/em5f.tim", 0},
	{"103a8891cdae8eac38a7c33bda9ebbc0","room/emd/em60.emd", 0},
	{"e7b52f10d424975099864ab88fd6d963","room/emd/em60.tim", 0},
	{"83811e5554f05d45743a908becfbb5cc","room/emd/em61.emd", 0},
	{"b1e85ed30a337ef650619018994aa2b0","room/emd/em61.tim", 0},
	{"fbd62743ad76cddec47b783507809e55","room/emd/em62.emd", 0},
	{"4f7972b7572fb9b2946c6a8fee3d1626","room/emd/em62.tim", 0},
	{"b6a14ccaba32f6eea0a26d67d8ee3865","room/emd/em63.emd", 0},
	{"e349e9d3650ebd34b30f0a5061c13fd3","room/emd/em63.tim", 0},
	{"3fe9ce5b0c51a5f0d73a67bb6ae8b3bf","room/emd/em64.emd", 0},
	{"e129c22479b5c8b9c839f39a2e959afe","room/emd/em64.tim", 0},
	{"018fe203e7861de009795ef04271c85e","room/emd/em65.emd", 0},
	{"ee7ffde6488972d391ae862ad7d9a49a","room/emd/em65.tim", 0},
	{"de0a2d754501781550af8193d9d2ba2a","room/emd/em66.emd", 0},
	{"f8ce609e6c4f7f8f9ad79e4ad9066ac8","room/emd/em66.tim", 0},
	{"8ec7f6fb2ba954aa9b155bd10a814415","room/emd/em67.emd", 0},
	{"c540b4355f0463220407c9b28bf76a05","room/emd/em67.tim", 0},
	{"c07ca5c8773e7ef30a6b197d6f977729","room/emd/em70.emd", 0},
	{"20bf8d887b0ab13de60ad034412d55f0","room/emd/em70.tim", 0},
	{"fbd62743ad76cddec47b783507809e55","room/emd/em71.emd", 0},
	{"53d47acf108f767564929a2507cc5616","room/emd/em71.tim", 0},

	{"e94c394189a7ba9838499a14831ab55f","room/emd08/em10.emd", 0},
	{"02613fdd2a9402284c7c906283d80973","room/emd08/em10.tim", 0},
	{"9319e8f07a511de51fe97122260915de","room/emd08/em11.emd", 0},
	{"0c29e033124028068ca56da61a19c4a7","room/emd08/em11.tim", 0},
	{"ed4662ca2fa969fd817c912df883c145","room/emd08/em12.emd", 0},
	{"7e04298f5a5adecb14b9d60c9b2d163e","room/emd08/em12.tim", 0},
	{"1097f3636e29f3636e9913419c0e86fe","room/emd08/em13.emd", 0},
	{"db00767cce785ef6da1c8e5755fbfe2f","room/emd08/em13.tim", 0},	/* = room/emd/em13.tim */
	{"909fe3a144469c5a54060c37818f3afa","room/emd08/em14.emd", 0},
	{"1225b45f80f7a991830d39ec23fd3e20","room/emd08/em14.tim", 0},
	{"6d4e63bdf331012f0efa85a7f0f1c7f3","room/emd08/em15.emd", 0},
	{"002bf41b7c8dab37cc177aba973b910f","room/emd08/em15.tim", 0},
	{"4de6fb76829ad287ef9bf5292e0e1462","room/emd08/em16.emd", 0},
	{"098a614f991bdaefdb706fac38900a89","room/emd08/em16.tim", 0},
	{"25e7b308734d35c3cd0ee49e914c1ae8","room/emd08/em17.emd", 0},
	{"a4acbb1a32c3301b9a722cf7f4d50f4a","room/emd08/em17.tim", 0},
	{"29295928b296beb71156436f1ffe45b8","room/emd08/em18.emd", 0},
	{"bd64827a53ac83376a37fb8540e42239","room/emd08/em18.tim", 0},
	{"5b94f8b24e43fc5833f4392d70f3dcfc","room/emd08/em19.emd", 0},
	{"d04ef240fd3157785bd3f46feabc0de6","room/emd08/em19.tim", 0},
	{"b9cdbbc179ffa4a7c025bb0cb9d8d511","room/emd08/em1a.emd", 0},
	{"47f3d7b46cc1e57d637ac328e17d7a26","room/emd08/em1a.tim", 0},
	{"e31177270c9b1ccca4ddaa6c0cf74705","room/emd08/em1b.emd", 0},
	{"d0188eb8a5e34bdd0f78a899e1b0bcc5","room/emd08/em1b.tim", 0},
	{"c4dc50f0c4d4a7f4ce4920eb772982c5","room/emd08/em1c.emd", 0},
	{"8f37dfd58aab16ea2bb67d68529d5a21","room/emd08/em1c.tim", 0},
	{"5238df561d645b7d94ca126c16a3468e","room/emd08/em1d.emd", 0},
	{"907c31e1adbe9807c256a4842cfb9a15","room/emd08/em1d.tim", 0},
	{"0f0a82e2e5c3cb26dac54e4e7f5bfa92","room/emd08/em1e.emd", 0},
	{"098a614f991bdaefdb706fac38900a89","room/emd08/em1e.tim", 0},
	{"aaf71ecbfb363a6c7a330a62a5e95a8a","room/emd08/em1f.emd", 0},
	{"fcb5c49f6ebba4330c0376ed65b751f9","room/emd08/em1f.tim", 0},
	{"371104bfcf44e95e5ada1af030755903","room/emd08/em20.emd", 0},
	{"b8ad398255ae64ebe1eebb77ce0984a9","room/emd08/em20.tim", 0},
	{"82bec8accce5a47bf44f5fa06a0ac55e","room/emd08/em21.emd", 0},
	{"413c9653df474b05f18f778a6cf5a1a2","room/emd08/em21.tim", 0},
	{"4db50dd8f49bd75a81239e0d96c582ff","room/emd08/em22.emd", 0},
	{"aeaf4bac057e0b19b23e85a707bf4631","room/emd08/em22.tim", 0},	/* = room/emd/em22.tim */
	{"ec0bba0099f7d28979006066589eccc8","room/emd08/em23.emd", 0},
	{"77d6dc3597fedd07dee5c2d4fddc3ab6","room/emd08/em23.tim", 0},
	{"e4e2f5539874d814e67903198d863393","room/emd08/em24.emd", 0},	/* = room/emd/em24.tim */
	{"6a5aae2906ea61bd978513811e7dc050","room/emd08/em24.tim", 0},
	{"97d0268b25e62a84d1dfe51498e04a4d","room/emd08/em25.emd", 0},
	{"33d63344a232dc5292c5e9d3188a293b","room/emd08/em25.tim", 0},
	{"84dde72548a5b980242bd268271d0f3c","room/emd08/em26.emd", 0},
	{"09a18a39781045f65c527c13ce6dbf3b","room/emd08/em26.tim", 0},
	{"d5afb99d8c826eec6519b6569616ac6f","room/emd08/em27.emd", 0},
	{"7514ff8bbcbaca75b2b87c2d3c61c343","room/emd08/em27.tim", 0},
	{"5c38f7a33b53507902638b93440591ab","room/emd08/em28.emd", 0},
	{"1dde25c43a8027bae6e3c221b88c909f","room/emd08/em28.tim", 0},
	{"a9a96c955f5b1fe804e1d93a3643e9a1","room/emd08/em32.emd", 0},
	{"a15eb28823b1463954861ba1100ca2b7","room/emd08/em32.tim", 0},
	{"140c686178a8ec05be929ffce0fbd2e1","room/emd08/em34.emd", 0},
	{"40c92c76cf55988fc8aaf6903fb74d6e","room/emd08/em34.tim", 0},
	{"d1cee2a89853c00c433b800dd42993eb","room/emd08/em36.emd", 0},
	{"f381d1228e1f293b4aba8815c77d0319","room/emd08/em36.tim", 0},
	{"cb2caf355f68ba54faac06d8824db816","room/emd08/em37.emd", 0},
	{"0166fe0b59d0ccd56ff6370a20d5cbe2","room/emd08/em37.tim", 0},
	{"1c6e07531be585a2cf09c2e27bca859c","room/emd08/em3a.emd", 0},
	{"9043a2e9d4921f44d172adc219a2e1d7","room/emd08/em3a.tim", 0},
	{"3bc3dac621d67b678141a3f606c303e2","room/emd08/em3b.emd", 0},
	{"0166fe0b59d0ccd56ff6370a20d5cbe2","room/emd08/em3b.tim", 0}
};

/*--- Variables ---*/

/* Extract files */
static int extract_files = 0;	

/* Extract for source code */
static int extract_src = 0;

/*--- Functions prototypes ---*/

int browse_iso(const char *filename);
int get_sector_size(SDL_RWops *src);
void extract_file(SDL_RWops *src, Uint32 start, Uint32 end, int block_size, int file_type);

Uint32 get_tim_length(Uint8 *buffer, Uint32 buflen);
Uint32 get_emd_length(Uint8 *buffer, Uint32 buflen);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s [-e] [-s] /path/to/filename.iso\n", argv[0]);
		return 1;
	}

	if (param_check("-e",argc,argv)>=0) {
		extract_files = 1;
	}
	if (param_check("-s",argc,argv)>=0) {
		extract_src = 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	retval = browse_iso(argv[argc-1]);

	SDL_Quit();
	return retval;
}

int browse_iso(const char *filename)
{
	SDL_RWops *src;
	Uint32 length, blocks, offset;
	Uint8 data[2352];
	int block_size, stop_extract=0;
	int i, extract_flag = 0, file_type = -1, new_file_type = -1;
	Uint32 start=0,end=0;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", filename);
		return 1;
	}

	block_size = get_sector_size(src);
	printf("Sector size: %d\n", block_size);

	start = end = 0;
	offset = (block_size == 2352) ? 16+8 : 8;
	for (i=0; !stop_extract; offset+=block_size, i++) {
		Uint32 value;

		SDL_RWseek(src, offset, RW_SEEK_SET);
		if (SDL_RWread(src, data, block_size, 1) != 1) {
			stop_extract=1;
			continue;
		}

		value = (data[3]<<24)|
			(data[2]<<16)|
			(data[1]<<8)|
			data[0];

		if (value == 0x00601408UL) {
			value = (data[4+3]<<24)|
				(data[4+2]<<16)|
				(data[4+1]<<8)|
				data[4];

			if (value == 0x00612408UL) {
				end = i;
				extract_flag = 1;
				/*new_file_type = FILE_DO3;*/
				new_file_type = -1;
			}
		} else if (value == MAGIC_TIM) {
			/* TIM image ? */
			value = (data[4+3]<<24)|
				(data[4+2]<<16)|
				(data[4+1]<<8)|
				data[4];

			switch(value) {
				case TIM_TYPE_4:
					end = i;
					extract_flag = 1;
					new_file_type = FILE_TIM_4;
					break;
				case TIM_TYPE_8:
					end = i;
					extract_flag = 1;
					new_file_type = FILE_TIM_8;
					break;
				case TIM_TYPE_16:
					end = i;
					extract_flag = 1;
					new_file_type = FILE_TIM_16;
					break;
			}
		} else if (value < 512<<10) {
			/* EMD model ? */
			value = (data[4+3]<<24)|
				(data[4+2]<<16)|
				(data[4+1]<<8)|
				data[4];

			if (value==0x0f) {
				end = i;
				extract_flag = 1;
				new_file_type = FILE_EMD;
			}
		} else if ((i-start)*block_size>=MAX_FILE_SIZE) {
			end = i;
			extract_flag = 1;
			new_file_type = -1;
		}

		if ((start!=0) && (end!=0) && extract_flag) {
			if (file_type != -1) {
				extract_file(src, start,end,block_size, file_type);
			}
			extract_flag = 0;
			file_type = new_file_type;
		}
		if (end!=0) {
			start = end;
			end = 0;
		}
	}

	SDL_RWclose(src);
	return 0;
}

int get_sector_size(SDL_RWops *src)
{
	char tmp[12];
	const char xamode[12]={0,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0};

	SDL_RWseek(src, 0, RW_SEEK_SET);
	SDL_RWread(src, tmp, 12, 1);
	if (memcmp(tmp, xamode, 12) != 0) {
		return 2336;
	}

	SDL_RWseek(src, 2352, RW_SEEK_SET);
	SDL_RWread(src, tmp, 12, 1);
	if (memcmp(tmp, xamode, 12) != 0) {
		return 2336;
	}

	return 2352;
}

void extract_file(SDL_RWops *src, Uint32 start, Uint32 end, int block_size, int file_type)
{
	Uint8 *buffer;
	Uint32 length = DATA_LENGTH * (end-start);
	int i, found;
	char filename[16];
	char *fileext = "%08x.bin";
	SDL_RWops *dst;

	md5_state_t state;
	md5_byte_t digest[16];
	char md5_file[32+1];

	buffer = (Uint8 *) malloc(length);
	if (!buffer) {
		return;
	}

	for (i=0; i<end-start; i++) {	
		if (block_size==2352) {
			SDL_RWseek(src, ((start+i)*2352)+16+8, RW_SEEK_SET);
		} else {
			SDL_RWseek(src, ((start+i)*2336)+8, RW_SEEK_SET);
		}
		SDL_RWread(src, &buffer[i*DATA_LENGTH], DATA_LENGTH, 1);
	}

	switch(file_type) {
		case FILE_TIM_4:
		case FILE_TIM_8:
		case FILE_TIM_16:
			fileext = "%08x.tim";
			length = get_tim_length(buffer, length);
			break;
		case FILE_EMD:
			fileext = "%08x.emd";
			length = get_emd_length(buffer, length);
			break;
		case FILE_DO3:
			fileext = "%08x.do3";
			/*length = get_emd_length(buffer, length);*/
			break;
	}
	sprintf(filename, fileext, start);

	/* Check MD5 for a known file */
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *) buffer, length);
	md5_finish(&state, digest);

	for (i=0; i<16; i++) {
		sprintf(&md5_file[i*2], "%02x", digest[i]);
	}

	found = -1;
	for (i=0; i<sizeof(md5_checks)/sizeof(md5_check_t); i++) {
		if (memcmp(md5_file, md5_checks[i].value, 32) != 0) {
			/* Not this md5, continue */
			continue;
		}

		if (md5_checks[i].found) {
			printf("File %s already dumped\n", md5_checks[i].filename);
			return;
		}

		/* Known md5 -> known file */
		md5_checks[i].found = 1;
		found = i;
		break;
	}

	switch(file_type) {
		case FILE_TIM_4:
			printf("Sector %d: 4 bits TIM image\n",start);
			break;
		case FILE_TIM_8:
			printf("Sector %d: 8 bits TIM image\n",start);
			break;
		case FILE_TIM_16:
			printf("Sector %d: 16 bits TIM image\n",start);
			break;
		case FILE_EMD:
			printf("Sector %d: EMD file\n",start);
			break;
		case FILE_DO3:
			printf("Sector %d: DO3 file\n",start);
			break;
	}

	if (extract_files) {
		dst = SDL_RWFromFile(filename, "wb");
		if (!dst) {
			fprintf(stderr, "Can not create %s for writing\n", filename);
			free(buffer);
			return;
		}

		SDL_RWwrite(dst, buffer, length, 1);
		SDL_RWclose(dst);
	}

	if (extract_src) {
		if (found == -1) {
			/*fprintf(stderr,"\t{%d,%d,\"%s\"},\n",start,end-start, filename);*/
			fprintf(stderr,"\t{%d,%d,\"\"},\n",start,end-start);
		} else {
			fprintf(stderr,"\t{%d,%d,\"%s\"},\n",start,end-start, md5_checks[i].filename);
		}
	}

	free(buffer);
}

Uint32 get_tim_length(Uint8 *buffer, Uint32 buflen)
{
	tim_header_t *tim_header = (tim_header_t *) buffer;
	tim_size_t *tim_size;
	int w,h, img_offset;

	img_offset = SDL_SwapLE32(tim_header->offset) + 20;

	tim_size = (tim_size_t *) (&((Uint8 *) buffer)[img_offset-4]);
	w = SDL_SwapLE16(tim_size->width);
	h = SDL_SwapLE16(tim_size->height);

	return img_offset+(w*h*2);
}

Uint32 get_emd_length(Uint8 *buffer, Uint32 buflen)
{
	Uint32 *emd_header = (Uint32 *) buffer;
	Uint32 dir_offset = SDL_SwapLE32(emd_header[0]);

	return dir_offset+4*15;
}
