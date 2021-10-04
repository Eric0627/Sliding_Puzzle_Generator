// export async function getAllUsers() {

//     const response = await fetch('/api/users');
//     return await response.json();
// }

export async function generatePuzzles(data) {
    console.log("Msg received in UserService.js",data)
    const response = await fetch(`/api/user`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({para: data})
    })
    return await response.json();
}