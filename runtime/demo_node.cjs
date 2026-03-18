exports.render = (payload) => {
  const name = payload?.name ?? "world";
  return `node:${name}`;
};
