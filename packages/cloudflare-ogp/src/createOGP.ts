import satori, { init } from 'satori/standalone';
import yogaWasm from 'satori/yoga.wasm';
import type { JSX } from 'react';
import { optimizeImage } from 'wasm-image-optimization';

await init(yogaWasm);

const cache = await caches.open('cloudflare-ogp2');

type Weight = 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900;
type FontStyle = 'normal' | 'italic';
type FontSrc = { data: ArrayBuffer | string; name: string; weight?: Weight; style?: FontStyle; lang?: string };
type Font = Omit<FontSrc, 'data'> & { data: ArrayBuffer | ArrayBufferView };

const downloadFont = async (name: string, weight: Weight) => {
	return await fetch(`https://fonts.googleapis.com/css2?family=${encodeURI(name)}:wght@${weight}`)
		.then((res) => res.text())
		.then((css) => css.match(/src: url\((.+)\) format\('(opentype|truetype)'\)/)?.[1])
		.then(async (url) => {
			return url !== undefined ? fetch(url).then((v) => (v.status === 200 ? v.arrayBuffer() : undefined)) : undefined;
		});
};

const getFonts = async (fontList: (string | [string, Weight])[], ctx: ExecutionContext): Promise<Font[]> => {
	const fonts: Font[] = [];
	for (const font of fontList) {
		const [fontName, weight] = Array.isArray(font) ? font : [font, 400 as Weight];
		const cacheKey = `http://font/${encodeURI(fontName)}-`;

		const response = await cache.match(cacheKey);
		if (response) {
			fonts.push({ name: fontName, data: await response.arrayBuffer(), weight, style: 'normal' });
		} else {
			const data = await downloadFont(fontName, weight);
			if (data) {
				ctx.waitUntil(cache.put(cacheKey, new Response(data)));
				fonts.push({ name: fontName, data, weight, style: 'normal' });
			}
		}
	}
	return fonts.flatMap((v): Font[] => (v ? [v] : []));
};

const createLoadAdditionalAsset = ({
	ctx,
	emojis,
}: {
	ctx: ExecutionContext;
	emojis: {
		url: string;
		upper?: boolean;
	}[];
}) => {
	const getEmojiSVG = async (code: string) => {
		const cacheKey = `http://emoji/${encodeURI(JSON.stringify(emojis))}/${code}`;
		for (const { url, upper } of emojis) {
			const emojiURL = `${url}${upper === false ? code.toLocaleLowerCase() : code.toUpperCase()}.svg`;
			let response = await cache.match(cacheKey);
			if (!response) {
				response = await fetch(emojiURL);
				if (response.status === 200) {
					ctx.waitUntil(cache.put(cacheKey, response.clone()));
				}
			}
			if (response.status === 200) {
				return await response.text();
			}
		}
		return undefined;
	};

	const loadEmoji = async (segment: string): Promise<string | undefined> => {
		const codes = Array.from(segment).map((char) => char.codePointAt(0));
		const isZero = codes.includes(0x200d);
		const code = codes
			.filter((code) => isZero || code !== 0xfe0f)
			.map((v) => v?.toString(16))
			.join('-');
		return getEmojiSVG(code);
	};

	const loadAdditionalAsset = async (code: string, segment: string) => {
		if (code === 'emoji') {
			const svg = await loadEmoji(segment);
			if (!svg) return segment;
			return `data:image/svg+xml;base64,${btoa(svg)}`;
		}
		return [];
	};

	return loadAdditionalAsset;
};

export const createOGP = async (
	element: JSX.Element,
	{
		fonts,
		emojis,
		ctx,
		width,
		height,
		scale = 1,
	}: {
		ctx: ExecutionContext;
		fonts: (string | [string, Weight])[];
		emojis?: {
			url: string;
			upper?: boolean;
		}[];
		width: number;
		height?: number;
		scale?: number;
	},
) => {
	const fontList = await getFonts(fonts, ctx);
	const svg = await satori(element, {
		width,
		height,
		fonts: fontList,
		loadAdditionalAsset: emojis ? createLoadAdditionalAsset({ ctx, emojis }) : undefined,
	});
	return svg;
	return await optimizeImage({
		image: svg,
		width: width * scale,
		height: height ? height * scale : undefined,
		format: 'png',
	}).then((v) => v.data);
};
